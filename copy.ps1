.\build.ps1

& adb push ./build/libbeatsaber-hook.so /sdcard/ModData/com.beatgames.beatsaber/Modloader/libs/libbeatsaber-hook.so
Copy-Item -Force build/llibbeatsaber-hook.so ../llibbeatsaber-hook.so