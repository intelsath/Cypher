#include "../libs/I2C.h"
#include "../libs/IMU.h"

extern "C"{
	#include <stdio.h>
	#include <string.h>
	#include <sys/fcntl.h>
	#include <sys/ioctl.h>
	#include <linux/random.h>
	#include <linux/types.h>
	#include <syslog.h>
}

#include <iostream>
#include <vector>

#define ENTROPY_SAMPS 1024

using namespace std;

// Read available entropy
void read_entropy()
{
	// Buffer
	int fd;
	int rd_bufsize = 1024; // Read a maximum 1024 bytes at a time
	char * buf = new char[rd_bufsize];
	fd = open("/proc/sys/kernel/random/entropy_avail", O_RDONLY);
	read( fd, buf , rd_bufsize ); // Read!
	close(fd);
	cout << "Available entropy: " << buf << endl;
	delete[] buf;
}

// Add entropy to /dev/random
int add_entropy(vector<uint8_t> buffer)
{
	// Read /proc/sys/kernel/random/entropy_avail
	read_entropy();

	struct {
	int entropy_count;
	int buf_size;
	char buf[ENTROPY_SAMPS];
	} entropy;

	cout << "Adding entropy... " << endl;

	// This is where entropy is added
	int count = ENTROPY_SAMPS;
	for( int i=0; i<count; i++ )
	{
		entropy.buf[i] = buffer[i];
		//cout << "entropy.buf[i]: ";
		//printf("%2x\n",buffer[i]);
 	}
		
    entropy.entropy_count = count * 8;
    entropy.buf_size = count;

	cout << "Opening /dev/random... " << endl;
	int randfd;
	if((randfd = open("/dev/random", O_WRONLY)) < 0) {
	perror("/dev/random");
	return 0;
	}

	cout << "Doing RNDADDENTROPY..." << endl;
	if (ioctl(randfd, RNDADDENTROPY, &entropy) != 0) {
		printf("ERROR: %s",strerror(errno));
		return 0;
	}

	read_entropy();

	return 1;

}

int main(){
	IMU imu_unit;

	vector<uint8_t> samples(ENTROPY_SAMPS); 
	int i = 0;
	while(1){
		// Sampling data
		int16_t Ax = imu_unit.get_output_acc('x');
		int16_t Ay = imu_unit.get_output_acc('y');
		int16_t Az = imu_unit.get_output_acc('z');
		int16_t Gx = imu_unit.get_output_gyr('x');
		int16_t Gy = imu_unit.get_output_gyr('y');
		int16_t Gz = imu_unit.get_output_gyr('z');

		imu_unit.update_acc_magnitute(Ax,Ay,Az); // Update magnitudes of accelerometer to determine if IMU is in motion
		bool motion_imu = imu_unit.is_moving();

		if( motion_imu )
		{
			cout << "Ax: " << Ax << " Ay: " << Ay << " Az: " << Az;
			cout << " Gx: " << Gx << " Gy: " << Gy << " Gz: " << Gz << endl;

			cout << "i: " << i << endl;
			// Add entropy to /dev/random
			if (i >= ENTROPY_SAMPS)
			{
				i = 0;
				add_entropy(samples);
			}

			// Get enough sampled entropy
			uint16_t mask = 0x00FF;
			samples[i] = (imu_unit.get_output_acc('x')&mask) ^ (imu_unit.get_output_gyr('x')&mask);
			i++;
			samples[i] = (imu_unit.get_output_acc('x')&(mask<<8)) ^ (imu_unit.get_output_gyr('x')&(mask<<8));
			i++;
			samples[i] = (imu_unit.get_output_acc('y')&mask) ^ (imu_unit.get_output_gyr('y')&mask);
			i++;
			samples[i] = (imu_unit.get_output_acc('y')&(mask<<8)) ^ (imu_unit.get_output_gyr('y')&(mask<<8));
			i++;
			samples[i] = (imu_unit.get_output_acc('z')&mask) ^ (imu_unit.get_output_gyr('z')&mask);
			i++;
			samples[i] = (imu_unit.get_output_acc('z')&(mask<<8)) ^ (imu_unit.get_output_gyr('z')&(mask<<8));
			i++;
			samples[i] = (imu_unit.get_output_acc('x')&mask) ^ (imu_unit.get_output_acc('y')&mask) ^ (imu_unit.get_output_acc('z')&mask);
			i++;
			samples[i] = (imu_unit.get_output_gyr('x')&(mask<<8)) ^ (imu_unit.get_output_gyr('y')&(mask<<8)) ^ (imu_unit.get_output_gyr('z')&(mask<<8));
			i++;
		}

		usleep(10000);
	}
	return 0;
}