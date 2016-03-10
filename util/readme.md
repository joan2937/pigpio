This folder provides utility files for the pigpio library.

### pigpiod

`pigpiod` is a script that allows to run pigpiod as a Linux service with root privileges.
The advantage of running pigpiod as a service is that pigpiod can be automatically launched on system startup.
To automatically start pigpiod as a service, do the following:

+ Copy `pigpiod` from this directory to `/etc/init.d/`.

+ Make it executable: 
  ```
  sudo chmod +x /etc/init.d/pigpiod
  ```

+ Tell update-rc.d to automatically start the pigpiod service on system startup:
  ```
  sudo update-rc.d pigpiod defaults
  ```
  
+ Now, you can start, stop, and restart pigpiod using
  ```
  sudo service pigpiod start
  sudo service pigpiod stop
  sudo service pigpiod restart
  ```


### Findpigpio.cmake

`Findpigpio.cmake` is a script used by CMake to find out where the pigpio header and library files are located.
