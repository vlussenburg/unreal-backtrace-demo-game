/home/ue4/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh \
 BuildCookRun \
 -utf8output \
 -platform=Linux \
 -clientconfig=Shipping \
 -serverconfig=Shipping \
 -project=/project/BacktraceGame.uproject \
 -noP4 -nodebuginfo -allmaps \
 -cook -build -stage -prereqs -pak -archive \
 -archivedirectory=/project/Packaged 
