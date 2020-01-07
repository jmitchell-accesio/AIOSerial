/*
//sample program to configure the port mode on ACCES PCI Serial Communications
//Products. 
//This sample is provided to show how to programaticcly configure the ports
//on ACCES PCI Serial Communications Products using sysfs which should be 
//available on any current Linux system.
//For more information about ACCES PCI Serial Communications cards please see 
//the README.md
//For more information on sysfs see man 5 sysfs or 
//https://en.wikipedia.org/wiki/Sysfs
//
*/



#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define PCI_VENDOR_ID_ACCESIO                   0x494f
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM_2SDB     0x1051
#define PCI_DEVICE_ID_ACCESIO_MPCIE_COM_2S      0x1053
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM_4SDB     0x105C
#define PCI_DEVICE_ID_ACCESIO_MPCIE_COM_4S      0x105E
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM232_2DB   0x1091
#define PCI_DEVICE_ID_ACCESIO_MPCIE_COM232_2    0x1093
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM232_4DB   0x1099
#define PCI_DEVICE_ID_ACCESIO_MPCIE_COM232_4    0x109B
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM_2SMDB    0x10D1
#define PCI_DEVICE_ID_ACCESIO_MPCIE_COM_2SM     0x10D3
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM_4SMDB    0x10DA
#define PCI_DEVICE_ID_ACCESIO_MPCIE_COM_4SM     0x10DC
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM485_1    0x1108
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM422_2    0x1110
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM485_2    0x1111
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM422_4    0x1118
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM485_4    0x1119
#define PCI_DEVICE_ID_ACCESIO_PCIE_ICM_2S       0x1152
#define PCI_DEVICE_ID_ACCESIO_PCIE_ICM_4S       0x115A
#define PCI_DEVICE_ID_ACCESIO_PCIE_ICM232_2     0x1190
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM232_2    0x1191
#define PCI_DEVICE_ID_ACCESIO_PCIE_ICM232_4     0x1198
#define PCI_DEVICE_ID_ACCESIO_MPCIE_ICM232_4    0x1199
#define PCI_DEVICE_ID_ACCESIO_PCIE_ICM_2SM      0x11D0
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM422_4     0x105A
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM485_4     0x105B
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM422_8     0x106A
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM485_8     0x106B
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM232_4     0x1098
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM232_8     0x10A9
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM_4SM      0x10D9
#define PCI_DEVICE_ID_ACCESIO_PCIE_COM_8SM      0x10E9
#define PCI_DEVICE_ID_ACCESIO_PCIE_ICM_4SM      0x11D8

#define SET_RS232 0
#define SET_RS422 1
#define SET_RS485_4W 0xb
#define SET_RS485_2W 0xf

#define UART_CONFIG 0xb4

//search /sys/devices for directories that contain ACCES pci devices
//writes a string for each path to dev_paths array. eg
//"/sys/devices/pci0000:00/0000:00:01.0/0000:01:00.0/"
//when calling send in NULL for path
void dev_paths_get(char dev_paths[][PATH_MAX], int max_paths, char *path)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char wd_path[PATH_MAX];
    int fd;
    char vid[8];
    bool init;
    static int count;
    char link_path[4];
    
    if (path == NULL)
    {
        init = true;
        path = wd_path;
        strcpy(path, "/sys/devices/");
    }

    if ((dp = opendir(path)) == NULL)
    {
        return;
    }

    chdir(path);

    while ((entry = readdir(dp)) != NULL)
    {
         lstat(entry->d_name, &statbuf);
         //S_ISLNK didn't work on test system. Since sysfs contains circular 
         //links we don't want to follow them.
         if(S_ISDIR(statbuf.st_mode) /*&& !(S_ISLNK(statbuf.st_mode))*/)
         {
             if(strcmp(".",entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                {
                    continue;
                }
            if (init)
            {
                if (strncmp(entry->d_name, "pci0000", 7) == 0)
                {
                    strcpy(wd_path, path);
                    strcat(wd_path, "/");
                    strcat(wd_path, entry->d_name);
                    dev_paths_get(dev_paths, max_paths, wd_path);
                }
            }
            else
            {
                    strcpy(wd_path, path);
                    strcat(wd_path, "/");
                    strcat(wd_path, entry->d_name);
                    //S_ISLINK() macro didn't work on test systems
                    //readlink() returns -1 if it's not a link
                    if (readlink(wd_path, link_path, sizeof(link_path)) == -1)
                        dev_paths_get(dev_paths, max_paths, wd_path);
            }
            
         }
         else
         {
             if (strcmp(entry->d_name, "vendor") == 0)
             {
                fd = open(entry->d_name, O_RDONLY);
                read(fd, vid, sizeof(vid));
                close(fd);
                //printf("Checking vid %s: \n", vid);
                if (strcmp(vid, "0x494f\n") == 0)
                {
                    strcpy(dev_paths[count], path);
                    strcat(dev_paths[count], "/");
                    count++;
                }
             }
         }
    }
    closedir(dp);
}

//looks up the card by pid. Expects the path to be an ACCES PCI device in sysfs.
//returns number of configurable ports.
int num_ports_get(char device_path[PATH_MAX])
{
    int j, fd;
    char pid_path[PATH_MAX];
    char pid[8];
    int ret_val = -1;

    strcpy(pid_path, device_path);
    strcat(pid_path, "/device");
    fd = open(pid_path, O_RDONLY);
    if (fd < 0)
    {
        printf("Could not open file: %s\n", pid_path);
        return ret_val;
    }
    read(fd, pid, sizeof(pid));
    close(fd);
    printf("Device located at device_path \"%s\"", device_path);

    switch(strtol(pid, NULL, 0))
    {
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM_2SDB:
            printf("PCIe-COM-2SDB: 2 port, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_COM_2S:
            printf("mPCIe-COM-2S: 2 port, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM_4SDB:
            printf("PCIe-COM-4SRJ: 4 port, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_COM_4S:
            printf("mPCIe-COM-4S: 4 port, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM232_2DB:
            printf("mPCIe-COM-4S: 4 port RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_COM232_2:
            printf("mPCIe-COM232-2: 2 port, RS-232 only\n");  
            ret_val = 0; break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM232_4DB:
            printf("PCIe-COM232-4DB: 2 port, RS-232 only\n");  
                ret_val = 0; 
                break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_COM232_4:
            printf("mPCIe-COM232-4: 2 port, RS-232 only\n");  
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM_2SMDB:
            printf("PCIe-COM-2SMDB: 2 port, RS-232, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_COM_2SM:
            printf("mPCIe-COM-2SM: 2 port, RS-232, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM_4SMDB:
            printf("PCIe-COM-4SMDB: 4 port, RS-232, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_COM_4SM:
            printf("mPCIe-COM-4SM: 4 port, RS-232, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM485_1: 
            printf("mPCIe-ICM485-1: 1 RS-485 only\n");
            ret_val = 0; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM422_2: 
            printf("mPCIe-ICM422-2: 2 port, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM485_2: 
            printf("mPCIe-ICM485-2: 2 port, RS-485 only.\n");
            ret_val = 0; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM422_4: 
            printf("mPCIe-ICM422-4: 4 port, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM485_4: 
            printf("mPCIe-ICM485-4: 4 port, RS-485 only\n");
            ret_val = 4;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_ICM_2S: 
            printf("PCIe-ICM-2S: 2 port, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_ICM_4S: 
            printf("PCIe-ICM-4S: 2 port, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_ICM232_2: 
            printf("PCIePCIe-ICM232-2: 2 port, RS-232 only\n");
            ret_val = 0; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM232_2: 
            printf("mPCIe-ICM232-2: 2 port, RS-232 only\n");
            ret_val = 0;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_ICM232_4:
            printf("PCIe-ICM232-4: 2 port, RS-232 only\n");
            ret_val = 0; 
            break;
        case PCI_DEVICE_ID_ACCESIO_MPCIE_ICM232_4:
            printf("mPCIe-ICM232-4: 4 port, RS-232 only\n");
            ret_val = 0;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_ICM_2SM: 
            printf("PCIe-ICM-2SM: 2 port, RS-232, RS-422, RS-485\n");
            ret_val = 2; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM422_4: 
            printf("PCIe-COM422-4: 2 port, RS-422, RS-485\n");
            ret_val = 4; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM485_4: 
            printf("PCIe-COM485-4: 4 port, RS-485 only\n");
            ret_val = 0;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM422_8: 
            printf("PCIe-COM422-8: 8 port, RS-422, RS-485\n");
            ret_val = 8; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM485_8: 
            printf("PCIe-COM485-8: 8 port, RS-485 only\n");
            ret_val = 8; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM232_4: 
            printf("PCIe-COM232-4: 4 port, RS-232 only\n");
            ret_val = 0;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM232_8: 
            printf("PCIe-COM232-8: 8 port, RS-232 only\n");
            ret_val = 8;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM_4SM: 
            printf("PCIe-COM-4SM: 4 port, RS-232, RS-422, RS-485\n");
            ret_val = 4;
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_COM_8SM: 
            printf("PCIe-COM-8SM: 8 port, RS-232, RS-422, RS-485\n");
            ret_val = 8; 
            break;
        case PCI_DEVICE_ID_ACCESIO_PCIE_ICM_4SM      : 
            printf("PCIe-ICM-4SM: 4 port, RS-232, RS-422, RS-485\n");
            ret_val = 0; 
            break;
        default:
            printf("Card not supported by this sample. Please visit https://accesio.com/ or contact tech@accesio.com\n");
            ret_val = 0;
            break;
    };
    return ret_val;
}
#define NUM_ENTRIES 6

int get_port_type (int i)
{
    bool done = false;
    char *line = NULL;
    size_t length = 0;
    int ret_val = 0;

    do
    {
        printf("Please enter type for port %c\n", 'A' + i);
        printf("1) RS-232\n");
        printf("2) RS-422\n");
        printf("3) RS-485(4W)\n");
        printf("4) RS-485(2W)\n");
        printf("=>");
        getline(&line, &length, stdin);
        ret_val = strtol(line, NULL, 0);
        free(line);
        if ((ret_val < 1) || (ret_val > 4))
        {
            line = NULL;
            length = 0;
        }
        else
        {
            done = true;
        }
        
    } while (done != true);

    switch (ret_val)
    {
        case 1:
            ret_val = SET_RS232;
            break;
        case 2:
            ret_val = SET_RS422;
            break;
        case 3:
            ret_val = SET_RS485_4W;
            break;
        case 4:
            ret_val = SET_RS485_2W;
            break;
        default:
            printf("default fault:%d\n", __LINE__);
            ret_val = 0;
            break;
    }

    return ret_val;
}

void config_card (char *dev_path, int ports)
{
    char *input = NULL;
    size_t len = 0;
    ssize_t read;
    char config_path[PATH_MAX];
    u_int8_t port_config;
    u_int8_t config_space[4] = {0};
    int i;
    int config_fd;


    for (i = 0; i < ports; i++)
    {
        port_config = get_port_type(i);

        //There are two layouts for the port configs on the chips supported by
        //this sample. One for the eight port cards and one for others. Each
        //port is a nybble
        if (ports == 8) //BA,CD,FE,HG
        {
            switch(i)
            {
                case 0: //A least significant nybble of byte zero
                    config_space[0] = (config_space[0] & 0xff00) | port_config;
                    break;
                case 1: //B most significant nybble of byte zero
                    config_space[0] = port_config  << 4 | (config_space[0] & 0xff);
                    break;
                case 2: //C most significant nybble of byte one
                    config_space[1] = port_config  << 4 | (config_space[1] & 0xff);
                    break;
                case 3: //D least significant nybble of byte one
                    config_space[1] = (config_space[1] & 0xff00) | port_config;
                    break;
                case 4: //E least significant nybble of byte two
                    config_space[2] = (config_space[0] & 0xff00) | port_config;
                    break;
                case 5: //F most significant nybble of byte two
                    config_space[2] = port_config  << 4 | (config_space[2] & 0xff);
                    break;
                case 6: //G least significant nybble of byte three
                    config_space[3] = (config_space[3] & 0xff00) | port_config;
                    break;
                case 7: //H most significant nybble of byte three
                    config_space[3] = port_config  << 4 | (config_space[3] & 0xff);
                    break;
                default:
                    printf("default fault:%d\n", __LINE__);
                    break;
            }
        }
        else //BA,xC,xx,Dx
        {
            switch(i)
            {
                case 0: //A least significant nybble of byte zero
                    config_space[0] |= (config_space[0] & 0xff00) | port_config;
                    break;
                case 1: //B most significant nybble of byte zero
                    config_space[0] |= port_config  << 4 | (config_space[0] & 0xff);
                    break;
                case 2: //C least significant nybble of byte one
                    config_space[1] |= ( config_space[1] & 0xff00) | port_config;
                    break;
                case 3: //D most significant nybble of byte three
                    config_space[3] |= port_config << 4 | (config_space[3] & 0xff);
                    break;
                default:
                    printf("default fault:%d\n", __LINE__);
                    break;
            }
        }
    }
    
    strcpy(config_path, dev_path);
    strcat(config_path, "/config");

    config_fd = open(config_path, O_RDWR);

    if (config_fd < 0)
    {
        printf("error opening config file: %s\n", strerror(errno));
        return;
    }

    if (lseek(config_fd, UART_CONFIG, SEEK_SET) == -1)
    {
        printf("error calling lseek: %s\n", strerror(errno));
        return;
    }

    if (write(config_fd, config_space, sizeof(config_space)) == -1)
    {
        printf("error writing to config: %s\n", strerror(errno));
        return;
    }

    close(config_fd);
}

int main (int argc, char **argv)
{
    char device_paths[NUM_ENTRIES][PATH_MAX] = {0};
    int i = 0;
    dev_paths_get(device_paths, NUM_ENTRIES, NULL);

    while ((device_paths[i][0] != '\0') && (i < NUM_ENTRIES))
    {
        printf("Found dev path at %s\n", device_paths[i]);
        config_card(device_paths[i], num_ports_get(device_paths[i]));
        i++;
    }
    return 0;
}
