EXE1 = intrinsic
SRC1 = calibrage_intrinseque.cpp

EXE2 = extrinsic
SRC2 = calibrage_extrinseque.cpp

extrinseque:
	g++ -o $(EXE2) -I"/Shared/TP_VTDVR/LIRIS-VISION" -I"/usr/include/opencv" -I"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks" $(SRC2) -L"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build" -L"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build" -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lcameraUVC -lcalibration -lcameraOPENCV -Xlinker -rpath="/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build:/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build:"

intrinseque:
	g++ -o $(EXE1) -I"/Shared/TP_VTDVR/LIRIS-VISION" -I"/usr/include/opencv" -I"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks" $(SRC1) -L"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build" -L"/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build" -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lcameraUVC -lcalibration -lcameraOPENCV -Xlinker -rpath="/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Apicamera/build:/Shared/TP_VTDVR/LIRIS-VISION/ModulesAndBricks/Calibration/build:"

clean:
	rm -rf *.o  *~
	
veryclean: clean
	rm -rf $(EXE1) $(EXE2)
