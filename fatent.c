/*
 * Copyright (C) 2004, OGAWA Hirofumi
 * Released under GPL v2.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/msdos_fs.h>
#include <linux/blkdev.h>
#include "fat.h"

//#define __DEBUG__
//#define __ORIGINAL_FAT_TEST__


struct super_block *temp_sb;

struct fatent_operations {
	void (*ent_blocknr)(struct super_block *, int, int *, sector_t *);
	void (*ent_set_ptr)(struct fat_entry *, int);
	int (*ent_bread)(struct super_block *, struct fat_entry *,
			 int, sector_t);
	int (*ent_get)(struct fat_entry *);
	void (*ent_put)(struct fat_entry *, int);
	int (*ent_next)(struct fat_entry *);
};

static DEFINE_SPINLOCK(fat12_entry_lock);

static void fat12_ent_blocknr(struct super_block *sb, int entry,
			      int *offset, sector_t *blocknr)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	int bytes = entry + (entry >> 1);
	WARN_ON(entry < FAT_START_ENT || sbi->max_cluster <= entry);
	*offset = bytes & (sb->s_blocksize - 1);
	*blocknr = sbi->fat_start + (bytes >> sb->s_blocksize_bits);
}

static void fat_ent_blocknr(struct super_block *sb, int entry,
			    int *offset, sector_t *blocknr)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	int bytes = (entry << sbi->fatent_shift);
	WARN_ON(entry < FAT_START_ENT || sbi->max_cluster <= entry);
	*offset = bytes & (sb->s_blocksize - 1);
	*blocknr = sbi->fat_start + (bytes >> sb->s_blocksize_bits);
}

static void fat12_ent_set_ptr(struct fat_entry *fatent, int offset)
{
	struct buffer_head **bhs = fatent->bhs;
	if (fatent->nr_bhs == 1) {
		WARN_ON(offset >= (bhs[0]->b_size - 1));
		fatent->u.ent12_p[0] = bhs[0]->b_data + offset;
		fatent->u.ent12_p[1] = bhs[0]->b_data + (offset + 1);
	} else {
		WARN_ON(offset != (bhs[0]->b_size - 1));
		fatent->u.ent12_p[0] = bhs[0]->b_data + offset;
		fatent->u.ent12_p[1] = bhs[1]->b_data;
	}
}

static void fat16_ent_set_ptr(struct fat_entry *fatent, int offset)
{
	WARN_ON(offset & (2 - 1));
	fatent->u.ent16_p = (__le16 *)(fatent->bhs[0]->b_data + offset);
}

static void fat32_ent_set_ptr(struct fat_entry *fatent, int offset)
{
	WARN_ON(offset & (4 - 1));
	fatent->u.ent32_p = (__le32 *)(fatent->bhs[0]->b_data + offset);
}

static int fat12_ent_bread(struct super_block *sb, struct fat_entry *fatent,
			   int offset, sector_t blocknr)
{
	struct buffer_head **bhs = fatent->bhs;

	WARN_ON(blocknr < MSDOS_SB(sb)->fat_start);
	fatent->fat_inode = MSDOS_SB(sb)->fat_inode;

	bhs[0] = sb_bread(sb, blocknr);
	if (!bhs[0])
		goto err;

	if ((offset + 1) < sb->s_blocksize)
		fatent->nr_bhs = 1;
	else {
		/* This entry is block boundary, it needs the next block */
		blocknr++;
		bhs[1] = sb_bread(sb, blocknr);
		if (!bhs[1])
			goto err_brelse;
		fatent->nr_bhs = 2;
	}
	fat12_ent_set_ptr(fatent, offset);
	return 0;

err_brelse:
	brelse(bhs[0]);
err:
	fat_msg(sb, KERN_ERR, "FAT read failed (blocknr %llu)", (llu)blocknr);
	return -EIO;
}

static int fat_ent_bread(struct super_block *sb, struct fat_entry *fatent,
			 int offset, sector_t blocknr)
{
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;

	WARN_ON(blocknr < MSDOS_SB(sb)->fat_start);
	fatent->fat_inode = MSDOS_SB(sb)->fat_inode;
	fatent->bhs[0] = sb_bread(sb, blocknr);
	if (!fatent->bhs[0]) {
		fat_msg(sb, KERN_ERR, "FAT read failed (blocknr %llu)",
		       (llu)blocknr);
		return -EIO;
	}
	fatent->nr_bhs = 1;
	ops->ent_set_ptr(fatent, offset);
	return 0;
}

static int fat12_ent_get(struct fat_entry *fatent)
{
	u8 **ent12_p = fatent->u.ent12_p;
	int next;

	spin_lock(&fat12_entry_lock);
	if (fatent->entry & 1)
		next = (*ent12_p[0] >> 4) | (*ent12_p[1] << 4);
	else
		next = (*ent12_p[1] << 8) | *ent12_p[0];
	spin_unlock(&fat12_entry_lock);

	next &= 0x0fff;
	if (next >= BAD_FAT12)
		next = FAT_ENT_EOF;
	return next;
}

static int fat16_ent_get(struct fat_entry *fatent)
{
	int next = le16_to_cpu(*fatent->u.ent16_p);
	WARN_ON((unsigned long)fatent->u.ent16_p & (2 - 1));
	if (next >= BAD_FAT16)
		next = FAT_ENT_EOF;
	return next;
}

static int fat32_ent_get(struct fat_entry *fatent)
{
	int next = le32_to_cpu(*fatent->u.ent32_p) & 0x0fffffff;

//	printk( KERN_ALERT "[cheon] fat32_ent_get \n");

	WARN_ON((unsigned long)fatent->u.ent32_p & (4 - 1));
	if (next >= BAD_FAT32)
	{
//		printk( KERN_ALERT "[cheon] fat32_ent_get, next = FAT_ENT_EOF, entry : %d\n", fatent->entry);

		next = FAT_ENT_EOF;
	}
	return next;
}

static void fat12_ent_put(struct fat_entry *fatent, int new)
{
	u8 **ent12_p = fatent->u.ent12_p;

	if (new == FAT_ENT_EOF)
		new = EOF_FAT12;

	spin_lock(&fat12_entry_lock);
	if (fatent->entry & 1) {
		*ent12_p[0] = (new << 4) | (*ent12_p[0] & 0x0f);
		*ent12_p[1] = new >> 4;
	} else {
		*ent12_p[0] = new & 0xff;
		*ent12_p[1] = (*ent12_p[1] & 0xf0) | (new >> 8);
	}
	spin_unlock(&fat12_entry_lock);

	mark_buffer_dirty_inode(fatent->bhs[0], fatent->fat_inode);
	if (fatent->nr_bhs == 2)
		mark_buffer_dirty_inode(fatent->bhs[1], fatent->fat_inode);
}

static void fat16_ent_put(struct fat_entry *fatent, int new)
{
	if (new == FAT_ENT_EOF)
		new = EOF_FAT16;

	*fatent->u.ent16_p = cpu_to_le16(new);
	mark_buffer_dirty_inode(fatent->bhs[0], fatent->fat_inode);
}

static void fat32_ent_put(struct fat_entry *fatent, int new)
{
	//cheon
	if( new == FAT_ENT_FREE )
	{
//		printk( KERN_ALERT "[cheon] fat32_ent_put, FAT_ENT_EFREE, %d \n", fatent->entry );
	}

	WARN_ON(new & 0xf0000000);
	new |= le32_to_cpu(*fatent->u.ent32_p) & ~0x0fffffff;
	*fatent->u.ent32_p = cpu_to_le32(new);
	mark_buffer_dirty_inode(fatent->bhs[0], fatent->fat_inode);
}

static int fat12_ent_next(struct fat_entry *fatent)
{
	u8 **ent12_p = fatent->u.ent12_p;
	struct buffer_head **bhs = fatent->bhs;
	u8 *nextp = ent12_p[1] + 1 + (fatent->entry & 1);

	fatent->entry++;
	if (fatent->nr_bhs == 1) {
		WARN_ON(ent12_p[0] > (u8 *)(bhs[0]->b_data +
							(bhs[0]->b_size - 2)));
		WARN_ON(ent12_p[1] > (u8 *)(bhs[0]->b_data +
							(bhs[0]->b_size - 1)));
		if (nextp < (u8 *)(bhs[0]->b_data + (bhs[0]->b_size - 1))) {
			ent12_p[0] = nextp - 1;
			ent12_p[1] = nextp;
			return 1;
		}
	} else {
		WARN_ON(ent12_p[0] != (u8 *)(bhs[0]->b_data +
							(bhs[0]->b_size - 1)));
		WARN_ON(ent12_p[1] != (u8 *)bhs[1]->b_data);
		ent12_p[0] = nextp - 1;
		ent12_p[1] = nextp;
		brelse(bhs[0]);
		bhs[0] = bhs[1];
		fatent->nr_bhs = 1;
		return 1;
	}
	ent12_p[0] = NULL;
	ent12_p[1] = NULL;
	return 0;
}

static int fat16_ent_next(struct fat_entry *fatent)
{
	const struct buffer_head *bh = fatent->bhs[0];
	fatent->entry++;
	if (fatent->u.ent16_p < (__le16 *)(bh->b_data + (bh->b_size - 2))) {
		fatent->u.ent16_p++;
		return 1;
	}
	fatent->u.ent16_p = NULL;
	return 0;
}

static int fat32_ent_next(struct fat_entry *fatent)
{
	const struct buffer_head *bh = fatent->bhs[0];
	fatent->entry++;
	if (fatent->u.ent32_p < (__le32 *)(bh->b_data + (bh->b_size - 4))) {
		fatent->u.ent32_p++;
		return 1;
	}
	fatent->u.ent32_p = NULL;
	return 0;
}

static struct fatent_operations fat12_ops = {
	.ent_blocknr	= fat12_ent_blocknr,
	.ent_set_ptr	= fat12_ent_set_ptr,
	.ent_bread	= fat12_ent_bread,
	.ent_get	= fat12_ent_get,
	.ent_put	= fat12_ent_put,
	.ent_next	= fat12_ent_next,
};

static struct fatent_operations fat16_ops = {
	.ent_blocknr	= fat_ent_blocknr,
	.ent_set_ptr	= fat16_ent_set_ptr,
	.ent_bread	= fat_ent_bread,
	.ent_get	= fat16_ent_get,
	.ent_put	= fat16_ent_put,
	.ent_next	= fat16_ent_next,
};

static struct fatent_operations fat32_ops = {
	.ent_blocknr	= fat_ent_blocknr,
	.ent_set_ptr	= fat32_ent_set_ptr,
	.ent_bread	= fat_ent_bread,
	.ent_get	= fat32_ent_get,
	.ent_put	= fat32_ent_put,
	.ent_next	= fat32_ent_next,
};

static inline void lock_fat(struct msdos_sb_info *sbi)
{
	mutex_lock(&sbi->fat_lock);
}

static inline void unlock_fat(struct msdos_sb_info *sbi)
{
	mutex_unlock(&sbi->fat_lock);
}

void fat_ent_access_init(struct super_block *sb)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	mutex_init(&sbi->fat_lock);

	switch (sbi->fat_bits) {
	case 32:
		sbi->fatent_shift = 2;
		sbi->fatent_ops = &fat32_ops;
		break;
	case 16:
		sbi->fatent_shift = 1;
		sbi->fatent_ops = &fat16_ops;
		break;
	case 12:
		sbi->fatent_shift = -1;
		sbi->fatent_ops = &fat12_ops;
		break;
	}
}

static void mark_fsinfo_dirty(struct super_block *sb)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	if (sb->s_flags & MS_RDONLY || sbi->fat_bits != 32)
		return;

	__mark_inode_dirty(sbi->fsinfo_inode, I_DIRTY_SYNC);
}

static inline int fat_ent_update_ptr(struct super_block *sb,
				     struct fat_entry *fatent,
				     int offset, sector_t blocknr)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct buffer_head **bhs = fatent->bhs;

	/* Is this fatent's blocks including this entry? */
	if (!fatent->nr_bhs || bhs[0]->b_blocknr != blocknr)
		return 0;
	if (sbi->fat_bits == 12) {
		if ((offset + 1) < sb->s_blocksize) {
			/* This entry is on bhs[0]. */
			if (fatent->nr_bhs == 2) {
				brelse(bhs[1]);
				fatent->nr_bhs = 1;
			}
		} else {
			/* This entry needs the next block. */
			if (fatent->nr_bhs != 2)
				return 0;
			if (bhs[1]->b_blocknr != (blocknr + 1))
				return 0;
		}
	}
	ops->ent_set_ptr(fatent, offset);
	return 1;
}

int fat_ent_read(struct inode *inode, struct fat_entry *fatent, int entry)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(inode->i_sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	int err, offset;
	sector_t blocknr;

	if (entry < FAT_START_ENT || sbi->max_cluster <= entry) {
		fatent_brelse(fatent);
		fat_fs_error(sb, "invalid access to FAT (entry 0x%08x)", entry);
		return -EIO;
	}

	fatent_set_entry(fatent, entry);
	ops->ent_blocknr(sb, entry, &offset, &blocknr);

	if (!fat_ent_update_ptr(sb, fatent, offset, blocknr)) {
		fatent_brelse(fatent);
		err = ops->ent_bread(sb, fatent, offset, blocknr);
		if (err)
			return err;
	}
	return ops->ent_get(fatent);
}

/* FIXME: We can write the blocks as more big chunk. */
static int fat_mirror_bhs(struct super_block *sb, struct buffer_head **bhs,
			  int nr_bhs)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *c_bh;
	int err, n, copy;

	err = 0;
	for (copy = 1; copy < sbi->fats; copy++) {
		sector_t backup_fat = sbi->fat_length * copy;

		for (n = 0; n < nr_bhs; n++) {
			c_bh = sb_getblk(sb, backup_fat + bhs[n]->b_blocknr);
			if (!c_bh) {
				err = -ENOMEM;
				goto error;
			}
			memcpy(c_bh->b_data, bhs[n]->b_data, sb->s_blocksize);
			set_buffer_uptodate(c_bh);
			mark_buffer_dirty_inode(c_bh, sbi->fat_inode);
			if (sb->s_flags & MS_SYNCHRONOUS)
				err = sync_dirty_buffer(c_bh);
			brelse(c_bh);
			if (err)
				goto error;
		}
	}
error:
	return err;
}

int fat_ent_write(struct inode *inode, struct fat_entry *fatent,
		  int new, int wait)
{
	struct super_block *sb = inode->i_sb;
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	int err;

	ops->ent_put(fatent, new);
	if (wait) {
		err = fat_sync_bhs(fatent->bhs, fatent->nr_bhs);
		if (err)
			return err;
	}
	return fat_mirror_bhs(sb, fatent->bhs, fatent->nr_bhs);
}

static inline int fat_ent_next(struct msdos_sb_info *sbi,
			       struct fat_entry *fatent)
{
	if (sbi->fatent_ops->ent_next(fatent)) {
		if (fatent->entry < sbi->max_cluster)
			return 1;
	}
	return 0;
}

static inline int fat_ent_read_block(struct super_block *sb,
				     struct fat_entry *fatent)
{
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int offset;

	fatent_brelse(fatent);
	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);
	return ops->ent_bread(sb, fatent, offset, blocknr);
}

static void fat_collect_bhs(struct buffer_head **bhs, int *nr_bhs,
			    struct fat_entry *fatent)
{
	int n, i;

	for (n = 0; n < fatent->nr_bhs; n++) {
		for (i = 0; i < *nr_bhs; i++) {
			if (fatent->bhs[n] == bhs[i])
				break;
		}
		if (i == *nr_bhs) {
			get_bh(fatent->bhs[n]);
			bhs[i] = fatent->bhs[n];
			(*nr_bhs)++;
		}
	}
}

int view_fatent_entry( void )
{
	struct msdos_sb_info *sbi = MSDOS_SB( temp_sb );
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent;
	int count, err, i;
	int entry = 0;

	unsigned long successive_data = 1;
	int temp_entry = 0;
	int flag = 0, start = 0, end = 0;
	/////////////////////////////////////////
	printk( KERN_ALERT "[cheon] view_fatent_entry() test \n");
	count = FAT_START_ENT;

	for( i = 1 ; i <= 4; i++ )
	{
		fatent_init( &fatent );
		fatent_set_entry( &fatent, sbi->bx_start_cluster[ i ] );

		while( count < sbi->max_cluster )
		{
			if( fatent.entry >= sbi->max_cluster )
				fatent.entry = FAT_START_ENT;

			fatent_set_entry( &fatent, fatent.entry );
			err = fat_ent_read_block( temp_sb, &fatent );
			if( err )
				goto out;

			do
			{
				if( ops->ent_get( &fatent ) == FAT_ENT_FREE )
				{
					entry = fatent.entry;
					if( entry == temp_entry + 1 )
					{
						successive_data++;

						if( flag == 0 ) start = temp_entry;

						end = entry;

						flag = 1;
					}   
					temp_entry = entry;
					//      printk( KERN_ALERT "[cheon] entry : %d \n", entry );
				}
				else
				{
					if( flag == 1 )
					{
						printk( KERN_ALERT "[cheon] [%d ~ %d ]\t: %luM \n",start, end, successive_data * 4096 / 1024 / 1024 );

						successive_data = 1;
						flag = 0;
					}   
				}   
				if( fatent.entry == sbi->bx_end_cluster[ i ] )
				{   
					if( flag == 1 )
					{
						printk( KERN_ALERT "[cheon] [%d ~ %d ]\t: %luM \n",start, end, successive_data * 4096 / 1024 / 1024  );

						successive_data = 1;
						flag = 0;
					}   
					goto out;
				}   
			}   
			while( fat_ent_next( sbi, &fatent ) );
		}   
out:    
		fatent_brelse( &fatent );
	}   

	return 0;
}   
EXPORT_SYMBOL_GPL( view_fatent_entry );





#if 0
/*
 모듈에서 호출해서 free된 공간 보는거임
 */
int view_fatent_entry( void )
{
	struct msdos_sb_info *sbi = MSDOS_SB( temp_sb );
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent;
	int count, err, i;
	
	printk( KERN_ALERT "[cheon] view_fatent_entry() \n");

	count = FAT_START_ENT;

	for( i = BB_NORMAL ; i <= BB_IMAGE ; i++ ) 
	{
		fatent_init( &fatent );
		fatent_set_entry( &fatent, sbi->bx_start_cluster[ i ] );

		while( count < sbi->max_cluster )
		{
			if( fatent.entry >= sbi->max_cluster )
				fatent.entry = FAT_START_ENT;

			fatent_set_entry( &fatent, fatent.entry );	
			err = fat_ent_read_block( temp_sb, &fatent );
			if( err )
				goto out;

			do{
				if( ops->ent_get( &fatent ) == FAT_ENT_FREE )		
				{
					int entry = fatent.entry;

					printk( KERN_ALERT "[cheon] entry : %d \n", entry );

				}	


				if( fatent.entry == sbi->bx_end_cluster[ i ] )
					goto out;

			}
			while( fat_ent_next( sbi, &fatent ) );
		}

out:

		fatent_brelse( &fatent );
	}

	return 0;

}
EXPORT_SYMBOL_GPL( view_fatent_entry );
#endif

int fat_alloc_cluster( struct inode *inode, int *cluster, int mode )
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent, prev_ent;
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	int i, count, err, nr_bhs, idx_clus;

	if(mode == -1){
		//get_area_number(&area,inode);
		printk("[cheon] Not 'etc' type file need one cluster allocation ! \n");
		return -1;
	}
//	else
//		area = mode;

	lock_fat(sbi);

	err = nr_bhs = idx_clus = 0;
	count = FAT_START_ENT;
	fatent_init(&prev_ent);
	fatent_init(&fatent);

	fatent_set_entry(&fatent, 0);

	while (count < sbi->max_cluster) {
		if (fatent.entry >= sbi->max_cluster)
			fatent.entry = FAT_START_ENT;
		else if(fatent.entry <  FAT_START_ENT)
			fatent.entry = FAT_START_ENT;
		//loop      

		fatent_set_entry(&fatent, fatent.entry);
		err = fat_ent_read_block(sb, &fatent);

		if (err)
			goto out;

		/* Find the free entries in a block */

		do {
			if (ops->ent_get(&fatent) == FAT_ENT_FREE) {
				int entry = fatent.entry;

				/* make the cluster chain */
				ops->ent_put(&fatent, FAT_ENT_EOF);
				if (prev_ent.nr_bhs)
					ops->ent_put(&prev_ent, entry);

				fat_collect_bhs(bhs, &nr_bhs, &fatent);

				sbi->prev_free = entry;
		//		sbi->bx_prev_free[area] = entry;
				//cheon
				//update for each area data
				if (sbi->free_clusters != -1){
					sbi->free_clusters--;
		//			sbi->bx_free_clusters[area]--;
				}
			//	sb->s_dirt = 1;

				cluster[idx_clus] = entry;
				idx_clus++;

				goto out;

			}

			count++;
			if (count == sbi->max_cluster)
				break;
		} while (fat_ent_next(sbi, &fatent));
		//if cat't find free cluster on current fat block
	}

out:
	unlock_fat(sbi);
	mark_fsinfo_dirty( sb );
	fatent_brelse(&fatent);
	if (!err) {
		if (inode_needs_sync(inode))
			err = fat_sync_bhs(bhs, nr_bhs);
		if (!err)
			err = fat_mirror_bhs(sb, bhs, nr_bhs);
	}
	for (i = 0; i < nr_bhs; i++)
		brelse(bhs[i]);

	if (err && idx_clus)
		fat_free_clusters(inode, cluster[0]);

	return err;
}

int fat_alloc_clusters(struct inode *inode, int *cluster, int nr_cluster)
{
	struct super_block *sb = inode->i_sb;
	temp_sb = sb;

	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent, prev_ent;
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	int i, count, err, nr_bhs, idx_clus;
	
//	struct dentry *dentry = NULL;
//	struct dentry *p_dentry = NULL;
	int area = 0; //proper work area number for inode
	unsigned long flags;

	BUG_ON(nr_cluster > (MAX_BUF_PER_PAGE / 2));	/* fixed limit */

	//printk( KERN_ALERT "[cheon] =================fat_alloc_clusters=============== \n");
	//[cheon]
	//Get File name & parent directory name
//	dentry = list_entry( inode->i_dentry.first, struct dentry, d_u.d_alias );
//	p_dentry = dentry->d_parent;

	//check area number from d_parent
//	get_area_number( &area, inode );

#if 0
#ifdef __ORIGINAL_FAT_TEST__
	if( area == BB_ETC )
		area = BB_ETC;
	else
		area = BB_NORMAL;
#endif
#endif
	lock_fat(sbi);
	if (sbi->free_clusters != -1 && sbi->free_clus_valid &&
	    sbi->free_clusters < nr_cluster) {
		printk( KERN_ALERT "[cheon] No space storage / current free %u / nr_cluster %d \n", sbi->free_clusters, nr_cluster );
		unlock_fat(sbi);
		return -ENOSPC;
	}
//#ifndef __ORIGINAL_FAT_TEST__

	if( sbi->fat_original_flag == OFF )
	{
		get_area_number( &area, inode );

		//printk("[cheon] get_area_number area : %d \n", area );

		//Free space check for each area
		spin_lock_irqsave( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
		if( sbi->bx_free_clusters[ area ] < nr_cluster )
		{
			printk( KERN_ALERT "[cheon] No space storage / bx current free %u / nr_cluster %d / area : %d  \n", sbi->bx_free_clusters[ area ], nr_cluster, area );
			unlock_fat(sbi);
			return -ENOSPC;
		}
		spin_unlock_irqrestore( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
	}

//#endif

//	full_sw = 0;
	err = nr_bhs = idx_clus = 0;
	count = FAT_START_ENT;
	fatent_init(&prev_ent);
	fatent_init(&fatent);
	
	if( sbi->fat_original_flag == ON )
		fatent_set_entry(&fatent, sbi->prev_free + 1);
	else
		fatent_set_entry(&fatent, sbi->bx_prev_free[area] + 0 );

	while (count < sbi->max_cluster) {
		if (fatent.entry >= sbi->max_cluster)
			fatent.entry = FAT_START_ENT;
		fatent_set_entry(&fatent, fatent.entry);
		err = fat_ent_read_block(sb, &fatent);
		if (err)
			goto out;

		/* Find the free entries in a block */
		do {

			if( sbi->fat_original_flag == ON ) 
			{
				if (ops->ent_get(&fatent) == FAT_ENT_FREE )
				{
					int entry = fatent.entry;

					/* make the cluster chain */
					ops->ent_put(&fatent, FAT_ENT_EOF);
					if (prev_ent.nr_bhs)
						ops->ent_put(&prev_ent, entry);


					fat_collect_bhs(bhs, &nr_bhs, &fatent);

					sbi->prev_free = entry;
//					sbi->bx_prev_free[ area ] = entry;

					//update for each area data
					if (sbi->free_clusters != -1)
					{
						sbi->free_clusters--;
		//				sbi->bx_free_clusters[ area ]--;
					}

					cluster[idx_clus] = entry;
					idx_clus++;
					if (idx_clus == nr_cluster)
						goto out;

					/*
					 * fat_collect_bhs() gets ref-count of bhs,
					 * so we can still use the prev_ent.
					 */
					prev_ent = fatent;
				}

			}
			else
			{
				if (ops->ent_get(&fatent) == FAT_ENT_FREE && sbi->bx_start_cluster[ area ] <= fatent.entry && sbi->bx_end_cluster[ area ] >= fatent.entry ) 
				{
					int entry = fatent.entry;

//					printk("%d ", entry );

					/* make the cluster chain */
					ops->ent_put(&fatent, FAT_ENT_EOF);
					if (prev_ent.nr_bhs)
						ops->ent_put(&prev_ent, entry);


					fat_collect_bhs(bhs, &nr_bhs, &fatent);

					sbi->prev_free = entry;
					sbi->bx_prev_free[ area ] = entry;

					//update for each area data
					if (sbi->free_clusters != -1)
					{
						spin_lock_irqsave( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
						sbi->free_clusters--;
						sbi->bx_free_clusters[ area ]--;
						spin_unlock_irqrestore( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
					}

					cluster[idx_clus] = entry;
					idx_clus++;
					if (idx_clus == nr_cluster)
						goto out;

					/*
					 * fat_collect_bhs() gets ref-count of bhs,
					 * so we can still use the prev_ent.
					 */
					prev_ent = fatent;
				}
			}
			count++;
			if (count == sbi->max_cluster)
				break;
		} while (fat_ent_next(sbi, &fatent));
	}

	/* Couldn't allocate the free entries */
	sbi->free_clusters = 0;
	sbi->free_clus_valid = 1;
	err = -ENOSPC;

out:
		
	unlock_fat(sbi);
	mark_fsinfo_dirty(sb);
	fatent_brelse(&fatent);
	if (!err) {
		if (inode_needs_sync(inode))
			err = fat_sync_bhs(bhs, nr_bhs);
		if (!err)
			err = fat_mirror_bhs(sb, bhs, nr_bhs);
	}
	for (i = 0; i < nr_bhs; i++)
	{
	//	printk( KERN_ALERT "[cheon] fat_alloc_clusters, b_blocknr : %lu \n", bhs[ i ]->b_blocknr );
		brelse(bhs[i]);
	}

	if (err && idx_clus)
	{
		printk( KERN_ALERT "[cheon] ==========fat_free_clusters test \n" );
		fat_free_clusters(inode, cluster[0]);
	}

#ifdef __DEBUG__
//	printk( KERN_ALERT "[cheon] fatent.entry : %d, sbi->free_clusters : %d, sbi->bx_free_clusters[ %d ] : %d  \n", fatent.entry, sbi->free_clusters, area, sbi->bx_free_clusters[area]  );
#endif
	return err;
}

static int get_area_number_for_free_func( struct inode *inode, int entry )
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	if( sbi->bx_start_cluster[ BB_ETC ] <= entry && entry <= sbi->bx_end_cluster[ BB_ETC ] )
		//(sbi->bx_free_clusters[ BB_ETC ])++;
		return BB_ETC;
	else if( sbi->bx_start_cluster[ BB_NORMAL ] <= entry && entry <= sbi->bx_end_cluster[ BB_NORMAL ] )
	//	(sbi->bx_free_clusters[ BB_NORMAL ])++;
		return BB_NORMAL;
	else if( sbi->bx_start_cluster[ BB_NORMAL_EVENT ] <= entry && entry <= sbi->bx_end_cluster[ BB_NORMAL_EVENT ] )
	//	(sbi->bx_free_clusters[ BB_NORMAL_EVENT ])++;
		return BB_NORMAL_EVENT;
	else if( sbi->bx_start_cluster[ BB_PARKING ] <= entry && entry <= sbi->bx_end_cluster[ BB_PARKING ] )
	//	(sbi->bx_free_clusters[ BB_PARKING ])++;
		return BB_PARKING;
	else if( sbi->bx_start_cluster[ BB_MANUAL ] <= entry && entry <= sbi->bx_end_cluster[ BB_MANUAL ] )
	//	(sbi->bx_free_clusters[ BB_MANUAL ])++;
		return BB_MANUAL;
	else if( sbi->bx_start_cluster[ BB_IMAGE ] <= entry && entry <= sbi->bx_end_cluster[ BB_IMAGE ] )
	//	(sbi->bx_free_clusters[ BB_IMAGE ])++;
		return BB_IMAGE;
	else
	{
		printk( KERN_ALERT "[cheon] ?????? get_area_number_for_free_func error \n" );
		return BB_ETC;
	}
}

int fat_free_clusters(struct inode *inode, int cluster)
{
	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent;
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	int i, err, nr_bhs;
	int first_cl = cluster, dirty_fsinfo = 0;
	int previous_cluster = 0;

	//choen
	int area=0;
	unsigned long flags;

	nr_bhs = 0;
	fatent_init(&fatent);
	lock_fat(sbi);

	//printk( KERN_ALERT "[cheon] fat_free_clusters\n" );

	do {
		cluster = fat_ent_read(inode, &fatent, cluster);
		if (cluster < 0) {
			err = cluster;
			goto error;
		} else if (cluster == FAT_ENT_FREE) {
			fat_fs_error(sb, "%s: deleting FAT entry beyond EOF",
				     __func__);
			err = -EIO;
			goto error;
		}

		//else
		//	printk( KERN_ALERT "%d ", cluster );
		//	printk( KERN_ALERT "[cheon] fat_free_clusters : %d ", cluster );

		if (sbi->options.discard) {
			/*
			 * Issue discard for the sectors we no longer
			 * care about, batching contiguous clusters
			 * into one request
			 */
			if (cluster != fatent.entry + 1) {
				int nr_clus = fatent.entry - first_cl + 1;

				sb_issue_discard(sb,
					fat_clus_to_blknr(sbi, first_cl),
					nr_clus * sbi->sec_per_clus,
					GFP_NOFS, 0);

				first_cl = cluster;
			}
		}

//		printk( KERN_ALERT "[cheon] fat_free_clusters \n");
#if 1 //origin
		ops->ent_put(&fatent, FAT_ENT_FREE);
		if (sbi->free_clusters != -1) {
			sbi->free_clusters++;

			if( sbi->fat_original_flag == OFF ) 
			{
					
			//	spin_lock_irqsave( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
#if 0
				#ifdef __ORIGINAL_FAT_TEST__ 
				if( sbi->bx_start_cluster[ BB_ETC ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_ETC ] )
					(sbi->bx_free_clusters[ BB_ETC ])++;
				else
					sbi->bx_free_clusters[ BB_NORMAL ]++;
				#endif
#endif
#if 1
				area = get_area_number_for_free_func( inode, fatent.entry );

				spin_lock_irqsave( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
				
				(sbi->bx_free_clusters[ area ])++;
			
#if 1
				if( cluster == FAT_ENT_EOF )				{
					sbi->bx_head[area] = previous_cluster + 1;
					printk("[cheon] fat_free_clusters, bx_head : %d \n", sbi->bx_head[area] );	

				}
				else 
					previous_cluster = cluster;		
#endif

				spin_unlock_irqrestore( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
#endif

#if 0
		//		#ifndef __ORIGINAL_FAT_TEST__
				if( sbi->bx_start_cluster[ BB_ETC ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_ETC ] )
				{
					(sbi->bx_free_clusters[ BB_ETC ])++;
			//		printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[BB_ETC] : %d \n", sbi->free_clusters, sbi->bx_free_clusters[ BB_ETC ] ); 
				}

				else if( sbi->bx_start_cluster[ BB_NORMAL ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_NORMAL ] )
				{
					(sbi->bx_free_clusters[ BB_NORMAL ])++;
			//		printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[BB_NORMAL] : %d \n", sbi->free_clusters, sbi->bx_free_clusters[ BB_NORMAL ] ); 
					//printk( KERN_ALERT "%d ", sbi->bx_free_clusters[ BB_NORMAL ] ); 
				//	printk( KERN_ALERT "%d ", fatent.entry ); 
				}

				else if( sbi->bx_start_cluster[ BB_NORMAL_EVENT ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_NORMAL_EVENT ] )
				{
					(sbi->bx_free_clusters[ BB_NORMAL_EVENT ])++;
			//		printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[BB_NORMAL_EVENT] : %d \n", sbi->free_clusters, sbi->bx_free_clusters[ BB_NORMAL_EVENT ] ); 
				}

				else if( sbi->bx_start_cluster[ BB_PARKING ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_PARKING ] )
				{
					(sbi->bx_free_clusters[ BB_PARKING ])++;
			//		printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[BB_PARKING] : %d \n", sbi->free_clusters, sbi->bx_free_clusters[ BB_PARKING ] ); 
				}

				else if( sbi->bx_start_cluster[ BB_MANUAL ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_MANUAL ] )
				{
					(sbi->bx_free_clusters[ BB_MANUAL ])++;
			//		printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[BB_MANUAL] : %d \n", sbi->free_clusters, sbi->bx_free_clusters[ BB_MANUAL ] ); 
				}

				else if( sbi->bx_start_cluster[ BB_IMAGE ] <= fatent.entry && fatent.entry <= sbi->bx_end_cluster[ BB_IMAGE ] )
				{
					(sbi->bx_free_clusters[ BB_IMAGE ])++;
			//		printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[BB_IMAGE] : %d \n", sbi->free_clusters, sbi->bx_free_clusters[ BB_IMAGE ] ); 
				}
				else
					printk( KERN_ALERT "[cheon] ?????? \n" );
		//		#endif

#endif
				#ifdef __DEBUG__
				//			printk( KERN_ALERT "[cheon] sbi->free_clusters : %d sbi->bx_free_clusters[2] : %d  \n", sbi->free_clusters, sbi->bx_free_clusters[2] );
				//			printk( KERN_ALERT "[cheon] fat_free_clusters , sbi->free_clusters : %d, fatent.entry : %d, sbi->bx_free_clusters[1] : %d \n", \
				sbi->free_clusters, fatent.entry, sbi->bx_free_clusters[1]  );
				//			printk( KERN_ALERT "[cheon] fatent.entry : %d ", fatent.entry );
				#endif
				
			//	spin_unlock_irqrestore( &MSDOS_SB(sb)->bx_lock[ area ], flags ); //cheon_lock
			}
			dirty_fsinfo = 1;
		}



#endif
		
		if (nr_bhs + fatent.nr_bhs > MAX_BUF_PER_PAGE) {
			if (sb->s_flags & MS_SYNCHRONOUS) {
				err = fat_sync_bhs(bhs, nr_bhs);
				if (err)
					goto error;
			}
			err = fat_mirror_bhs(sb, bhs, nr_bhs);
			if (err)
				goto error;
			for (i = 0; i < nr_bhs; i++)
				brelse(bhs[i]);
			nr_bhs = 0;
		}
		fat_collect_bhs(bhs, &nr_bhs, &fatent);
	} while (cluster != FAT_ENT_EOF);

	if (sb->s_flags & MS_SYNCHRONOUS) {
		err = fat_sync_bhs(bhs, nr_bhs);
		if (err)
			goto error;
	}
	err = fat_mirror_bhs(sb, bhs, nr_bhs);
error:
	fatent_brelse(&fatent);
	for (i = 0; i < nr_bhs; i++)
		brelse(bhs[i]);
	unlock_fat(sbi);
	if (dirty_fsinfo)
		mark_fsinfo_dirty(sb);

	return err;
}
EXPORT_SYMBOL_GPL(fat_free_clusters);

/* 128kb is the whole sectors for FAT12 and FAT16 */
#define FAT_READA_SIZE		(128 * 1024)

static void fat_ent_reada(struct super_block *sb, struct fat_entry *fatent,
			  unsigned long reada_blocks)
{
	struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int i, offset;

	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);

	for (i = 0; i < reada_blocks; i++)
		sb_breadahead(sb, blocknr + i);
}

void get_area_number( int *area, struct inode *inode )
{
	struct dentry *dentry = NULL;
	struct dentry *upper_dentry = NULL;
	int temp_area = -1;
	////////////////////
	//FOR SD Card 7/26
	//struct super_block *sb = inode->i_sb;

	//printk( "[cheon] S_ID : %s\n", sb->s_id  );

	
	dentry = list_entry( inode->i_dentry.first, struct dentry, d_u.d_alias ); 

	if( S_ISDIR( inode->i_mode) || dentry->d_parent == NULL)
	{
		temp_area = BB_ETC;

//		printk( "[cheon] DE, alloc ETC \n");
	}	
	else
	{
		upper_dentry = dentry->d_parent;

		while( upper_dentry != NULL )
		{
			if( upper_dentry->d_name.name == NULL )
				break;

//			if(strcmp(upper_dentry->d_name.name, ETC_DIRECTORY ) == 0 )
//				temp_area = BB_ETC;
			//etc는 따로 디렉터리 설정으로 하지 않고 말그대로 기타로 하겟음 ( ex ) 현재 보드상엣 /media/boot 부분에 boot.ini랑 디바이스 파일이랑 , zImage올라가있는데
			//이 쪽 파티션이 vfat로 되어 있어서 이런 역할로 etc를 나눠 주겟음

			else if(strcmp(upper_dentry->d_name.name, DIR_1 ) == 0 )
				temp_area = NUM_1;

			else if(strcmp(upper_dentry->d_name.name, DIR_2 ) == 0 )
				temp_area = NUM_2;
			
			else if(strcmp(upper_dentry->d_name.name, DIR_3 ) == 0 )
				temp_area = NUM_3;

			else if(strcmp(upper_dentry->d_name.name, DIR_4 ) == 0 )
				temp_area = NUM_4;

			else if(strcmp(upper_dentry->d_name.name, DIR_5 ) == 0 )
				temp_area = NUM_5;
			else //test
				temp_area = BB_ETC;


			if( temp_area != -1 )
				break;
			else if( !strcmp("/", upper_dentry->d_name.name ) )
				break;
			else
				upper_dentry =upper_dentry->d_parent;
		}
	}

	if( 1 <= temp_area && temp_area <= 4 ) //normal, evnet, parking, manual인데 avi가 아니먄 etc에서 할당
	{
		if( strstr( dentry->d_name.name, "avi" ) == NULL )
		{
			*area = BB_ETC;			
			//printk("[cheon] get_area_number(nut avi) :%d \n",*area);
			return;
		}		
	}

	if( temp_area == -1 )
		*area = BB_ETC;
	else
		*area =temp_area;
	
//	printk("[cheon] get_area_number : %d\n", *area );
}

int fat_count_free_clusters_for_area(struct super_block *sb)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent;
	unsigned long reada_blocks, reada_mask, cur_block;
	int free;

	reada_blocks = FAT_READA_SIZE >> sb->s_blocksize_bits;
	reada_mask = reada_blocks - 1;
	cur_block = 0;

	free = 0;
	fatent_init(&fatent);
	fatent_set_entry(&fatent, FAT_START_ENT);

	if( sbi->bx_free_valid != -1 )
		goto area_out;


	while (fatent.entry < sbi->max_cluster) {
		/* readahead of fat blocks */
		if ((cur_block & reada_mask) == 0) {
			unsigned long rest = sbi->fat_length - cur_block;
			fat_ent_reada(sb, &fatent, min(reada_blocks, rest));
		}
		cur_block++;

		fat_ent_read_block(sb, &fatent);

		do {
			if (ops->ent_get(&fatent) == FAT_ENT_FREE)
			{
				free++;

			//cheon		
				if( fatent.entry < sbi->bx_start_cluster[ BB_ETC ] );

				else if( fatent.entry <= sbi->bx_end_cluster[ BB_ETC ] )
					(sbi->bx_free_clusters[ BB_ETC ])++;
		
				else if( fatent.entry <= sbi->bx_end_cluster[ BB_NORMAL ] )
					(sbi->bx_free_clusters[ BB_NORMAL ])++;

				else if( fatent.entry <= sbi->bx_end_cluster[ BB_NORMAL_EVENT ] )
					(sbi->bx_free_clusters[ BB_NORMAL_EVENT ])++;

				else if( fatent.entry <= sbi->bx_end_cluster[ BB_PARKING ] )
					(sbi->bx_free_clusters[ BB_PARKING ])++;
		
				else if( fatent.entry <= sbi->bx_end_cluster[ BB_MANUAL ] )
					(sbi->bx_free_clusters[ BB_MANUAL ])++;
		
				else if( fatent.entry <= sbi->bx_end_cluster[ BB_IMAGE ] )
					(sbi->bx_free_clusters[ BB_IMAGE ])++;
		


				//else;

			}

		} while (fat_ent_next(sbi, &fatent));
	}

	sbi->bx_free_valid = 1;

	sbi->free_clusters = free;
	sbi->free_clus_valid = 1;
	mark_fsinfo_dirty(sb);
	fatent_brelse(&fatent);

area_out:
	return 0;
}


int fat_count_free_clusters(struct super_block *sb)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct fatent_operations *ops = sbi->fatent_ops;
	struct fat_entry fatent;
	unsigned long reada_blocks, reada_mask, cur_block;
	int err = 0, free;

	lock_fat(sbi);
	if (sbi->free_clusters != -1 && sbi->free_clus_valid)
		goto out;

	reada_blocks = FAT_READA_SIZE >> sb->s_blocksize_bits;
	reada_mask = reada_blocks - 1;
	cur_block = 0;

	free = 0;
	fatent_init(&fatent);
	fatent_set_entry(&fatent, FAT_START_ENT);
	while (fatent.entry < sbi->max_cluster) {
		/* readahead of fat blocks */
		if ((cur_block & reada_mask) == 0) {
			unsigned long rest = sbi->fat_length - cur_block;
			fat_ent_reada(sb, &fatent, min(reada_blocks, rest));
		}
		cur_block++;

		err = fat_ent_read_block(sb, &fatent);
		if (err)
			goto out;

		do {
			if (ops->ent_get(&fatent) == FAT_ENT_FREE)
			{
				printk( KERN_ALERT "fat_count_free_clusters free++ \n" );
				free++;
			}
		} while (fat_ent_next(sbi, &fatent));
	}
	sbi->free_clusters = free;
	sbi->free_clus_valid = 1;
	mark_fsinfo_dirty(sb);
	fatent_brelse(&fatent);
out:
	unlock_fat(sbi);
	return err;
}
