EXE1=intrinsic
SRC1 = calibrage_intrinseque.cpp

EXE1=extrinsic
SRC1 = calibrage_extrinseque.cpp

all:
	g++ -o $(EXE2) -I"/Shared/TP_VTDVR/LIRIS-VISION" -I"/usr/include/opencv" -I"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks" $(SRC2) -L"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build" -L"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build" -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lcameraUVC -lcalibration -lcameraOPENCV -Xlinker -rpath="/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build:/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build:"

clean:
	rm -rf *.o $(EXE1) $(EXE2) *~
