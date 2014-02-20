SRMA
====

System Resource Monitoring Application

### 동작 방식 ###
1. drivers 파일 컴파일
2. *.ko파일을 adb push 명령어를 통해 Android Kernel로 복사한다.
3. 안드로이드 shell에서 su권한 획득 후 insmod *.ko 명령어로 driver module 등록한다.
4. chmod 0666 driver_name 으로 권한 설정한다.
5. 사용 후 rmmod driver_name으로 module을 내린다.

#### ver1.0(20140221) ####
- drivers/cpuinfo - cpu register 정보를 갖고 오는 character device driver module
- app/cpuinfo_test.tar.gz - driver moudle test application
