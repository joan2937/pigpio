#!/usr/bin/env python

from distutils.core import setup

setup(name='pigpio',
      version='1.25',
      author='joan',
      author_email='joan@abyz.co.uk',
      maintainer='joan',
      maintainer_email='joan@abyz.co.uk',
      url='http://abyz.co.uk/rpi/pigpio/python.html/',
      description='Raspberry gpio module',
      long_description='Raspberry Python module to access the pigpio daemon',
      download_url='http://abyz.co.uk/rpi/pigpio/pigpio.zip',
      license='unlicense.org',
      py_modules=['pigpio']
     )
