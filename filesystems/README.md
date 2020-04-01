# File system drivers

## Source

  https://linux-kernel-labs.github.io/refs/heads/master/labs/filesystems_part1.html
  
  https://linux-kernel-labs.github.io/refs/heads/master/labs/filesystems_part2.html

## Result
*myfs* module is a virtual file system (no disk support). After we implemented all the necessary components (**file_system_type, superblock,...**), we could do some operators in this new filesystem as *mount/unmount*, some basic file operations as *mkdir, mknod, touch,...* even read, write a file in our filesystem.
![](https://user-images.githubusercontent.com/61527787/78162600-4d95a280-7471-11ea-96ca-5a9de72ed660.png)
