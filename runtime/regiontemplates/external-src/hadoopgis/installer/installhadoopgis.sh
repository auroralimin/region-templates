#! /bin/bash

echo "Installing Dependencies..."
sudo apt-get install libboost-all-dev
bash ../dependencies/ubuntu_install_geos.sh
bash ../dependencies/ubuntu_install_libspatialindex.sh
echo "Dependencies Instaled"

# Configuring lib and include directories
usage(){
  echo -e "installhadoopgis.sh [options]\n \
  -l PATH, --libpath=PATH \t [Optional] Directory path to the lib locations of dependencies (include lib)\n \
  -i PATH, --incpath=PATH \t [Optional] Directory path to the include locations"
  exit 1
}

# Default empty values
libpath=""
incpath=""

while :
do
    case $1 in
        -h | --help | -\?)
          usage;
          exit 0
          ;;
        -l | --libpath)
          libpath=$2
          shift 2
          ;;
        --libpath=*)
          libpath=${1#*=}
          shift
          ;;
        -i | --incpath)
          incpath=$2
          shift 2
          ;;
        --incpath=*)
          incpath=${1#*=}
          shift
          ;;
        --)
          shift
          break
          ;;
        -*)
          echo "Unknown option: $1" >&2
          shift
          ;;
        *) # Done
          break
          ;;
    esac
done

# Create a directory for binary/executables
mkdir -p "$( dirname "${BASH_SOURCE[0]}")/../build"



if [ ! "$incpath" ] && [ ! "$libpath"]; then
	incpath="$( cd "$( dirname "${BASH_SOURCE[0]}")/../built/include" && pwd)"
	libpath="$( cd "$( dirname "${BASH_SOURCE[0]}")/../built/lib" && pwd)"
	binpath="$( cd "$( dirname "${BASH_SOURCE[0]}")/../build/bin" && pwd)"
#  incpath="$( cd "$( dirname "${BASH_SOURCE[0]}/../built/include" )" && pwd )"
#  libpath="$( cd "$( dirname "${BASH_SOURCE[0]}/../built/lib" )" && pwd )"
fi
if [ ! "$incpath" ] ; then
  echo "ERROR: Missing include path. See --help" >&2
  exit 1
fi
if [ ! "$libpath" ] ; then
  echo "ERROR: Missing lib (library) path. See --help" >&2
  exit 1
fi

# Cd and running individual makefiles
echo $incpath
echo $libpath
HADOOPGIS_INC_PATH=$incpath
HADOOPGIS_LIB_PATH=$libpath

export HADOOPGIS_BIN_PATH=$binpath
export HADOOPGIS_INC_PATH=$incpath
export HADOOPGIS_LIB_PATH=$libpath

echo "export HADOOPGIS_INC_PATH=$incpath" >> ~/.bashrc
echo "export HADOOPGIS_LIB_PATH=$libpath" >> ~/.bashrc

# Save the paths
echo "HADOOPGIS_INC_PATH=${HADOOPGIS_INC_PATH}" > ../hadoopgis.cfg
echo "HADOOPGIS_LIB_PATH=${HADOOPGIS_LIB_PATH}" >> ../hadoopgis.cfg

# cd to the directory where installHADOOPGIS.sh is located.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
cd $SCRIPT_DIR

echo "Installing HadoopGIS in build folder..."
cd ../src
make install
make

# Return to the install directory
cd ${SCRIPT_DIR}

echo "Done with installation"