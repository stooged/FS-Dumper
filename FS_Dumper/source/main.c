#include "ps4.h"
#include "patch.h"


int nthread_run;
char notify_buf[1024];


void copyDir(char *sourcedir, char* destdir)
{
    DIR *dir;
    struct dirent *dp;
    struct stat info;
    char src_path[1024], dst_path[1024];
    dir = opendir(sourcedir);
    if (!dir)
        return;
        mkdir(destdir, 0777);
    while ((dp = readdir(dir)) != NULL)
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") || !strcmp(dp->d_name, "app") || !strcmp(dp->d_name, "mnt"))
        {}
        else
        {
            sprintf(src_path, "%s/%s", sourcedir, dp->d_name);
            sprintf(dst_path, "%s/%s", destdir  , dp->d_name);
            if (!stat(src_path, &info))
            {
                if (S_ISDIR(info.st_mode))
                {
                  copyDir(src_path, dst_path);
                }
                else
                if (S_ISREG(info.st_mode))
                {
                    if(is_self(src_path))
                    {
                        decrypt_and_dump_self(src_path, dst_path);
                    }  
                    else
                    {
                        copy_File(src_path, dst_path);
                    }
                }
            }
        }
    }
    closedir(dir);
}

void mntFs()
{
	mount_fs("/dev/da0x0.crypt", "/preinst",   "exfatfs", "511", UPDATE_MNT);
	mount_fs("/dev/da0x1.crypt", "/preinst2",  "exfatfs", "511", UPDATE_MNT);
	mount_fs("/dev/da0x4.crypt", "/system",    "exfatfs", "511", UPDATE_MNT);
	mount_fs("/dev/da0x5.crypt", "/system_ex", "exfatfs", "511", UPDATE_MNT);
}

void *nthread_func(void *arg)
{
        time_t t1, t2;
        t1 = 0;
	while (nthread_run)
	{
		if (notify_buf[0])
		{
			t2 = time(NULL);
			if ((t2 - t1) >= 10)
			{
				t1 = t2;
                systemMessage(notify_buf);
			}
		}
		else t1 = 0;
		sceKernelSleep(1);
	}
	return NULL;
}


int _main(struct thread *td) {
	initKernel();
	initLibc();
	initPthread();
	syscall(11,patcher,td);
    initSysUtil();
    nthread_run = 1;
    notify_buf[0] = '\0';
    ScePthread nthread;
    scePthreadCreate(&nthread, NULL, nthread_func, NULL, "nthread");
	int usbdir = open("/mnt/usb0/.dirtest", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (usbdir == -1)
	{
        usbdir = open("/mnt/usb1/.dirtest", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (usbdir == -1)
		{
			systemMessage("No Usb Found.");
			nthread_run = 0;
			return 0;
		}
		else
		{
			close(usbdir);
			unlink("/mnt/usb1/.dirtest");
			systemMessage("Dumping to USB1");
			mkdir("/mnt/usb1/FS_DUMP/", 0777);
			mntFs();
			sprintf(notify_buf, "Dumping: Data\nPlease wait.");
			copyDir("/", "/mnt/usb1/FS_DUMP");
			systemMessage("Filesystem Dump Complete.");
			notify_buf[0] = '\0';
			nthread_run = 0;
		}
	}
	else
	{
		close(usbdir);
		unlink("/mnt/usb0/.dirtest");
		systemMessage("Dumping to USB0");
		mkdir("/mnt/usb0/FS_DUMP/", 0777);
		mntFs();
		sprintf(notify_buf, "Dumping: Data\nPlease wait.");
		copyDir("/", "/mnt/usb0/FS_DUMP");
		systemMessage("Filesystem Dump Complete.");
		notify_buf[0] = '\0';
		nthread_run = 0;
	}
	
	return 0;
}