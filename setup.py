#!/usr/bin/env python

from setuptools import setup, find_packages

setup(
    name='pigpio',
    version='1.34',
    author='joan',
    author_email='joan@abyz.co.uk',
    maintainer='joan',
    maintainer_email='joan@abyz.co.uk',
    url='http://abyz.co.uk/rpi/pigpio/python.html',
    description='Raspberry Pi GPIO module',
    long_description='Raspberry Python module to access the pigpio daemon',
    download_url='http://abyz.co.uk/rpi/pigpio/pigpio.zip',
    license='unlicense.org',
    keywords=[
        'raspberrypi',
        'gpio',
    ],
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
    ]
)
