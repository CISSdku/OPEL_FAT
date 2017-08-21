#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/pagemap.h>
#include <linux/mpage.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/parser.h>
#include <linux/uio.h>
#include <linux/writeback.h>
#include <linux/log2.h>
#include <linux/hash.h>
#include <asm/unaligned.h>
#include "fat.h"

//const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =30\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =10\r\n\tBX_MANUAL       =5\r\n\tBX_IMAGE        =5\r\n\tBX_ETC          =30\r\n\r\nPreallocation Setting[MB]\r\n\tBX_NORMAL      =20\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =20\r\n\tBX_MANUAL      =20\r\n\t\0";
//const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =40\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =10\r\n\tBX_MANUAL       =5\r\n\tBX_IMAGE        =5\r\n\tBX_ETC          =20\r\n\r\nPreallocation Setting[MB]\r\n\tBX_NORMAL      =60\r\n\tBX_NORMAL_EVENT    =60\r\n\tBX_PARKING_EVENT   =60\r\n\tBX_MANUAL      =60\r\n\t\0";
const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =40\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =10\r\n\tBX_MANUAL       =5\r\n\tBX_IMAGE        =5\r\n\tBX_ETC          =20\r\n\r\nPreallocation Setting[MB]\r\n\tBX_NORMAL      =32\r\n\tBX_NORMAL_EVENT    =32\r\n\tBX_PARKING_EVENT   =32\r\n\tBX_MANUAL      =32\r\n\t\0";
//const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =40\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =10\r\n\tBX_MANUAL       =5\r\n\tBX_IMAGE        =5\r\n\tBX_ETC          =20\r\n\r\nPreallocation Setting[MB]\r\n\tBX_NORMAL      =80\r\n\tBX_NORMAL_EVENT    =60\r\n\tBX_PARKING_EVENT   =48\r\n\tBX_MANUAL      =48\r\n\t\0";
//const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =40\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =10\r\n\tBX_MANUAL       =5\r\n\tBX_IMAGE        =5\r\n\tBX_ETC          =20\r\n\r\nPreallocation Setting[MB]\r\n\tBX_NORMAL      =48\r\n\tBX_NORMAL_EVENT    =48\r\n\tBX_PARKING_EVENT   =48\r\n\tBX_MANUAL      =48\r\n\t\0";
//const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =70\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =7\r\n\tBX_MANUAL       =2\r\n\tBX_IMAGE        =2\r\n\tBX_ETC          =7\r\n\r\nPreallocation Setting[MB]\r\n\tBX_NORMAL      =28\r\n\tBX_NORMAL_EVENT    =28\r\n\tBX_PARKING_EVENT   =28\r\n\tBX_MANUAL      =28\r\n\t\0";
//const char *config_data = "Blackbox Configuration\r\n\r\nPartitioning Size[Percentage]\r\n\tBX_NORMAL       =70\r\n\tBX_NORMAL_EVENT    =20\r\n\tBX_PARKING_EVENT   =7\r\n\tBX_MANUAL       =2\r\n\tBX_IMAGE        =2\r\n\tBX_ETC          =7\r\n\r\n";

unsigned char msdos_name[ MSDOS_NAME ] = "BXFS_CON";

static int read_config_data(struct super_block *sb, char *data)
{
	/*
	   Read Data from config file.
	   Need to change more smart
	 */
	struct msdos_sb_info *sbi = MSDOS_SB(sb);

	int num_of_pre = 4;
	int num_of_par = 6;
	int num_of_total = num_of_pre + num_of_par;
//	int num_of_total = num_of_par;

	int i;
	int block_size = 512;
	int count = 0;

	int read_val[20] = {0, };

	for(i=0; i<block_size; i++)
	{
		//printk("%c", data[i]);
		if(data[i] == '='){
			if(data[i+3] >= '0' && data[i+3] <= '9'){//Case1 : 3 digit number
				read_val[count] = (((int)(data[i+1])-48) * 100) +  (((int)(data[i+2])-48) * 10) +  (((int)(data[i+3])-48)) ;
				i+=3;
			}
			else if(data[i+2] >= '0' && data[i+2] <= '9'){ //Case 2 : 2 digit number
				read_val[count] = (((int)(data[i+1])-48) * 10) +  (((int)(data[i+2])-48)) ;
				i+=2;
			}
			else if(data[i+1] >= '0' && data[i+1] <= '9'){ //Case 3 : 1 digit number
				read_val[count] = (((int)(data[i+1])-48)) ;
				i++;
			}
			else{
				printk("Invalid config file data! \n");
				return -1;
			}

			count++;

			if(count >= num_of_total){
				printk("[cheon] Complete data parsing \n");
				break;
			}

		}
	}

	if(count < num_of_total){
		printk("Some config date are dropped! \n");
		return -1;
	}

	//  unsigned int bx_area_ratio[10];
	//  unsigned int bx_pre_size[10]

#if 0
	sbi->bx_area_ratio[BX_NORMAL]           = read_val[0];
	sbi->bx_area_ratio[BX_NORMAL_EVENT]     = read_val[1];
	sbi->bx_area_ratio[BX_PARKING_EVENT]    = read_val[2];
	sbi->bx_area_ratio[BX_MANUAL]           = read_val[3];
	sbi->bx_area_ratio[BX_IMAGE]            = read_val[4];
	sbi->bx_area_ratio[BX_ETC]          	= read_val[5];
#endif

	sbi->bx_area_ratio[ BB_NORMAL		 ]    = read_val[0];
	sbi->bx_area_ratio[ BB_NORMAL_EVENT  ]    = read_val[1];
	sbi->bx_area_ratio[ BB_PARKING 		 ]    = read_val[2];
	sbi->bx_area_ratio[ BB_MANUAL 		 ]    = read_val[3];
	sbi->bx_area_ratio[ BB_IMAGE         ]    = read_val[4];
	sbi->bx_area_ratio[ BB_ETC 			 ] 	  = read_val[5];

#if 1
	printk("[cheon] %u \n", sbi->bx_area_ratio[ BB_NORMAL ] );
	printk("[cheon] %u \n", sbi->bx_area_ratio[ BB_NORMAL_EVENT ] );
	printk("[cheon] %u \n", sbi->bx_area_ratio[ BB_PARKING ] );
	printk("[cheon] %u \n", sbi->bx_area_ratio[ BB_MANUAL ] );
	printk("[cheon] %u \n", sbi->bx_area_ratio[ BB_IMAGE ] );
	printk("[cheon] %u \n\n", sbi->bx_area_ratio[ BB_ETC ] );
#endif
	

#if 0
	sbi->bx_pre_size[BX_NORMAL]             = read_val[6];
	sbi->bx_pre_size[BX_NORMAL_EVENT]       = read_val[7];
	sbi->bx_pre_size[BX_PARKING_EVENT]      = read_val[8];
	sbi->bx_pre_size[BX_MANUAL]             = read_val[9];
#endif
#if 1
	sbi->bx_pre_size[ BB_NORMAL ]    	  = read_val[6];
	sbi->bx_pre_size[ BB_NORMAL_EVENT ]   = read_val[7];
	sbi->bx_pre_size[ BB_PARKING ]		  = read_val[8];
	sbi->bx_pre_size[ BB_MANUAL ]  		  = read_val[9];

	printk("[cheon] %u \n", sbi->bx_pre_size[ BB_NORMAL ] );
    printk("[cheon] %u \n", sbi->bx_pre_size[ BB_NORMAL_EVENT ] ); 
    printk("[cheon] %u \n", sbi->bx_pre_size[ BB_PARKING ] );	
	printk("[cheon] %u \n",	sbi->bx_pre_size[ BB_MANUAL ] );
	
#endif
	return 0;
	//one more time, check config file
}

static int _fat_chain_add(struct inode *inode, int new_dclus, int nr_cluster)
{

	struct super_block *sb = inode->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	int ret, new_fclus, last;

	/*
	 * We must locate the last cluster of the file to add this new
	 * one (new_dclus) to the end of the link list (the FAT).
	 */
	last = new_fclus = 0;

	/* add new one to the last of the cluster chain */

	MSDOS_I(inode)->i_start = new_dclus;
	MSDOS_I(inode)->i_logstart = new_dclus;

	/*
	 * Since generic_write_sync() synchronizes regular files later,
	 * we sync here only directories.
	 */

	//sync wirte for fat scan
	ret = fat_sync_inode(inode);
	if (ret)
		return ret;

	/*
	   if (S_ISDIR(inode->i_mode) && IS_DIRSYNC(inode)) {
	   ret = fat_sync_inode(inode);
	   if (ret)
	   return ret;
	   } else
	   mark_inode_dirty(inode);
	   printk("[gandan] Phase 8-3 \n");
	 */

	inode->i_blocks = nr_cluster << (sbi->cluster_bits - 9);

	return 0;
}

int build_config_file( struct inode *dir, struct dentry *dentry, int mode )
{
	struct msdos_dir_entry de;
	struct fat_slot_info sinfo;
	struct fat_slot_info *_sinfo = &sinfo;
	struct inode *inode = NULL;
	
	struct super_block *sb = dir->i_sb;
	struct msdos_sb_info *sbi = MSDOS_SB( sb );
	
	int cluster_num = 0;
	int err;
	int cluster = sbi->fat_start;

//	struct dentry *root_de = sb->s_root;
//	struct inode *root = root_de->d_inode;
#if 0
	if( !fat_scan( dir, msdos_name, &sinfo ) )
	{
		printk("[cheon] build_config_file : existing file ");
		brelse( sinfo.bh );
		err = -EINVAL;
		goto out;
	}
#endif

	//First, Create empty file
	memcpy( de.name, msdos_name, MSDOS_NAME ); //msdos_dir_entry의 name에 msdos_name배열 안에 이름을

	de.attr = 0 ? ATTR_DIR : ATTR_ARCH;
	de.lcase = 0;
	de.cdate = de.adate = 0;
	de.ctime = 0;
	de.ctime_cs = 0;
	de.time = 0;
	de.date = 0;
	de.start = cpu_to_le16(cluster_num);
	de.starthi = cpu_to_le16(cluster_num) >> 16;  //high 16bit니깐 이게 맞는거 같은데 원래 괄호안에 >> 16이 있었음
	de.size = 512; //length of data

	err = fat_add_entries( dir, &de, 1, _sinfo ); 			//int fat_add_entries(struct inode *dir, void *slots, int nr_slots,struct fat_slot_info *sinfo)
	inode = fat_build_inode( sb, sinfo.de, sinfo.i_pos );

	brelse( sinfo.bh );

	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		printk("[cheon] Error about inode...\n");
		goto out;
	}
	printk("[cheon] Complete Create Config File \n");
//////////////////	
#if 0
	if( !fat_scan( root, msdos_name, &sinfo ) )
	{
		brelse( sinfo.bh );	
		printk("[cheon] config file already exist \n");

	}
#endif
	
	// Second Cluster allocation
	if (!err){
		printk("[cheon] start fat_flush_inodes : %u \n", cluster);
		err = fat_flush_inodes(sb, dir, inode);

		//alloc speific area
		printk("[cheon] start fat_alloc_cluster : %u \n", cluster);
		fat_alloc_cluster(inode, &cluster, BX_ETC);

		printk("[cheon] get %d cluster \n", cluster);
		_fat_chain_add( inode, cluster, 1 );

	}
	else
		printk("[gandan] Error or Skipping \n");


	printk("[gandan] Complete cluster allocation for config file \n");

out:
	return err;
}

#if 0
static inline sector_t fat_clus_to_blknr(struct msdos_sb_info *sbi, int clus)
{       
	return ((sector_t)clus - FAT_START_ENT) * sbi->sec_per_clus	+ sbi->data_start;
}   
#endif

static int opel_vfat_find_form( struct msdos_sb_info *sbi, struct inode *dir, unsigned char *name, sector_t *blknr )
{
	struct fat_slot_info sinfo;
	int err = fat_scan_opel(dir, name, &sinfo);	
	if (err)
		    return -ENOENT;
	else 
		printk("[cheon] detect \n");

	sector_t clus = sinfo.de->start;

	brelse(sinfo.bh);
	printk("[cheon] sinfo's start info : %u \n", clus );

//	*blknr = fat_clus_to_blknr( sbi, sinfo.de->start ); 

#if 0 //여기서 하니깐 파일 이름이 바뀌네 //그럼 일단 여기까지는 맞는다는건데
	strcpy( sinfo.bh->b_data, "test" );

	set_buffer_uptodate( sinfo.bh );
	mark_buffer_dirty_inode( sinfo.bh, dir );
#endif


 	*blknr = ( (clus - FAT_START_ENT) * sbi->sec_per_clus ) + sbi->data_start;
	return 0;
}

/*
 	Update for pre-allocation config file setting
 */
int fat_config_init(struct super_block *sb)
{
	struct fat_slot_info sinfo;
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct dentry *root_de = sb->s_root;
	struct inode *root = root_de->d_inode;

	struct buffer_head *bh;
	unsigned int block_pos;
	int err;

	sector_t blknr;
	
	printk("[cheon] Start fat_config_init \n");

#if 1
	if( !fat_scan_opel( root, msdos_name, &sinfo ) )
	{
		printk("[cheon] config file already exist \n");
		brelse( sinfo.bh );	

		goto exist;
	}
#endif

	//First - Create file
	printk("[cheon] Create config File \n");
	err = build_config_file( root, root_de, 0 );
/////////////////////////////////////

	//Second - Data Input
	if( opel_vfat_find_form( sbi, root, msdos_name, &blknr ) == -ENOENT )
		printk("[cheon] opel_vfat_find_form error \n");


//	printk("[cheon] blknr : %u \n", blknr );

//	blknr = 16402;

#if 1
	bh = sb_bread( sb, blknr );
	if( !bh )
	{
		printk("[cheon] unable to read inode block  \n");	
		brelse(bh);
	}

	strcpy( bh->b_data, config_data );

	mark_buffer_dirty(bh);
	err = sync_dirty_buffer(bh);
	brelse(bh);

	printk("[cheon] Complete write data \n");

	if (fat_scan(root, msdos_name, &sinfo)) 
	{
		printk("[cheon] <==Error==> Creation config file failed! \n");
		return -1;
	} 
#endif


#if 0
	bh = sb_getblk( sb, blknr );
	if( !bh )
	{
		printk("[cheon] sb_getblk error \n");
	}
	strcpy( bh->b_data,  config_data ); 
//	memset( (void *)bh->b_data, 0x0, sizeof( 512 ) );

//	strcpy( bh->b_data, "test" );
//	strcpy( sbi->bh->b_data, "test" );

//j	set_buffer_uptodate( bh );
//	mark_buffer_dirty_inode( bh, root );


	mark_buffer_dirty(bh);
	err = sync_dirty_buffer(bh);
//	brelse(bh);

	if (fat_scan(root, msdos_name, &sinfo)) 
	{
		printk("[gandan] <==Error==> Creation config file failed! \n");
		return -1;
	} 
#endif
exist:	
	//Data read and Set sb info	

	//printk("[cheon] Data Read \n");

//	block_pos = ((sinfo.de->start - 2) * sbi->sec_per_clus ) + sbi->data_start ;

#if 1	
	if( opel_vfat_find_form( sbi, root, msdos_name, &blknr ) == -ENOENT )
		printk("[cheon] opel_vfat_find_form error \n");

//	printk("[cheon] blknr : %u \n", blknr );
	bh = sb_bread( sb, blknr );	
	read_config_data( sb, bh->b_data );
	brelse( bh );
#endif
	printk("[cheon] Start data read...complete\n");

	return 0;
}
EXPORT_SYMBOL_GPL( fat_config_init );



































