#!/bin/bash
# Check if the .env file exists
if [[ -f .env ]]; then
    # Load environment variables from the .env file
    export $(cat .env | grep -v '^#' | xargs)
else
    # Error
    echo "Error"
    exit 1
fi

# Some executables were not found. Compile!
make all 
MAKE_EXIT_CODE=$?
if [[  -z $MAKE_EXIT_CODE ]]; then
	echo $MAKE_EXIT_CODE
	echo "Make Failed"
	exit 1
fi

# launch vars displayer
setsid --fork gnome-terminal -- sudo $PWD/varsDisplayer -d $SEM_VARS -v $MEM_VARS -n $N_TASKS

# launch webCamCapture
setsid --fork gnome-terminal -- sudo $PWD/webCamCapture -s $SEM_WEBCAM -m $MEM_WEBCAM -v $WEB_CAM -d

echo "Dont Forget to press enter to launch main after running the first two terminals\n"
read

# launch Main program
setsid --fork gnome-terminal -- sudo $PWD/main -x $WIDTH -y $HEIGHT -s $SEM_WEBCAM -m $MEM_WEBCAM -d $SEM_VARS -v $MEM_VARS -n $N_TASKS
