git submodule update --init --force --recursive

IF EXIST "%OBS_STUDIO_32_FILE%" (
	curl -kL "%OBS_STUDIO_32_URL%" -f --retry 5 -o "%OBS_STUDIO_32_FILE%" -z "%OBS_STUDIO_32_FILE%"
) ELSE (
	curl -kL "%OBS_STUDIO_32_URL%" -f --retry 5 -o "%OBS_STUDIO_32_FILE%"
)
7z x -y "%OBS_STUDIO_32_FILE%" -o. > nul

IF EXIST "%OBS_STUDIO_64_FILE%" (
	curl -kL "%OBS_STUDIO_64_URL%" -f --retry 5 -o "%OBS_STUDIO_64_FILE%" -z "%OBS_STUDIO_64_FILE%"
) ELSE (
	curl -kL "%OBS_STUDIO_64_URL%" -f --retry 5 -o "%OBS_STUDIO_64_FILE%"
)
7z x -y "%OBS_STUDIO_64_FILE%" -o. > nul

IF "%APPVEYOR_REPO_TAG%"=="true" (
  SET "PACKAGE_NAME=%PACKAGE_PREFIX%-%APPVEYOR_REPO_TAG_NAME%"
) ELSE (
	IF "%APPVEYOR_PULL_REQUEST_NUMBER%"=="" (
		SET "PACKAGE_NAME=%PACKAGE_PREFIX%-%APPVEYOR_REPO_BRANCH%-%APPVEYOR_BUILD_NUMBER%"
	) ELSE (
		SET "PACKAGE_NAME=%PACKAGE_PREFIX%-%APPVEYOR_PULL_REQUEST_NUMBER%-%APPVEYOR_BUILD_NUMBER%"
	)
)

IF EXIST inno.exe (
	curl -kL "%INNOSETUP_URL%" -f --retry 5 -o inno.exe -z inno.exe
) else (
	curl -kL "%INNOSETUP_URL%" -f --retry 5 -o inno.exe
)
inno.exe /VERYSILENT /NORETART /SP- /SUPPRESSMSGBOXES