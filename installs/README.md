# Install Requirements

## Python

Python is usually pre-installed for Mac and Linux, but on windows, run 'python-3.7.4.exe' and then follow these steps:

1. Open the installer and click on 'Customize Installation'
2. Click next (optionally you can de-select 'Documentation' check-box)
3. Ensure 'Add Python to environmental variables' is checked

## SCons

Installation of `scons` requires python to be invokable on your command-line. Python gives you `pip` and you can use it to install `scons`:

1. Open up a terminal window and ensure python is installed: `python --version`
2. `pip install scons`

### Install Scons manually

If you successfully installed using the steps above, then you do not have to follow this section.  The `scons-3.1.0.zip` is provided in case you want to install it manually, which can be done by doing:

1. Extract the zip file
2. `cd` into the zip file and then `python setup.py install`


## Clang Format (Windows only)

Clang Format for Windows may require VC runtime libraries. Install `vc_redistx64.exe` if you encounter the error:
```
The program can't start because MSVCP140.dll is missing from your computer.
```
