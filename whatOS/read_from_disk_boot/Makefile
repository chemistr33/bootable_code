all:
	nasm -f bin ./boot_read_from_disk.asm -o ./boot.bin
	dd if=./disk_data.txt >> ./boot.bin
	dd if=/dev/zero bs=512 count=1 >> ./boot.bin

clean:
	rm -f ./boot.bin
