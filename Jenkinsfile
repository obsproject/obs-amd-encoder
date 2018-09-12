pipeline {
  agent any
  stages {
    stage('Initialize') {
      steps {
        sh 'git submodule update --init --recursive --force'
        sh '''curl -kL "${INNOSETUP_URL}" -f --retry 5 -o inno.exe
./inno.exe //SP- //VERYSILENT //SUPPRESSMSGBOXES //NOCANCEL //NORESTART'''
      }
    }
    stage('Configure') {
      parallel {
        stage('32-Bit') {
          steps {
            cmake(installation: 'default', arguments: '-H. -B"build/32" -G"Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" -DCMAKE_PACKAGE_PREFIX="${CMAKE_PACKAGE_PREFIX}" -DCMAKE_PACKAGE_NAME="${CMAKE_PACKAGE_NAME}"')
          }
        }
        stage('64-Bit') {
          steps {
            cmake(installation: 'default', arguments: '-H. -B"build/64" -G"Visual Studio 15 2017 Win64" -T"host=x64" -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" -DCMAKE_PACKAGE_PREFIX="${CMAKE_PACKAGE_PREFIX}" -DCMAKE_PACKAGE_NAME="${CMAKE_PACKAGE_NAME}"')
          }
        }
      }
    }
    stage('Build') {
      parallel {
        stage('32-bit') {
          steps {
            cmake(installation: 'default', arguments: '--build build/32 --target ALL_BUILD --config RelWithDebInfo')
          }
        }
        stage('64-bit') {
          steps {
            cmake(installation: 'default', arguments: '--build build/64 --target ALL_BUILD --config RelWithDebInfo')
          }
        }
      }
    }
    stage('Package') {
      steps {
        cmake(installation: 'default', arguments: '--build build/32 --target INSTALL --config RelWithDebInfo')
        cmake(installation: 'default', arguments: '--build build/64 --target INSTALL --config RelWithDebInfo')
        cmake(installation: 'default', arguments: '--build build/64 --target PACKAGE_ZIP --config RelWithDebInfo')
        cmake(installation: 'default', arguments: '--build build/64 --target PACKAGE_7Z --config RelWithDebInfo')
        sh '"C:\\Program Files (x86)\\Inno Setup 5\\ISCC.exe" //Qp "build/64/ci/installer.iss"'
      }
    }
    stage('Artifacts') {
      steps {
        archiveArtifacts(artifacts: 'build/*.exe', onlyIfSuccessful: true, allowEmptyArchive: true)
        archiveArtifacts(artifacts: 'build/*.7z', allowEmptyArchive: true, onlyIfSuccessful: true)
        archiveArtifacts(artifacts: 'build/*.zip', allowEmptyArchive: true, onlyIfSuccessful: true)
      }
    }
  }
  environment {
    CMAKE_SYSTEM_VERSION = '10.0.17134.0'
    PACKAGE_PREFIX = 'amd-encoder-for-obs-studio'
    INNOSETUP_URL = 'http://www.jrsoftware.org/download.php/is-unicode.exe'
    CMAKE_INSTALL_PREFIX = 'build/distrib'
    CMAKE_PACKAGE_NAME = 'obs-amd-encoder'
    CMAKE_PACKAGE_PREFIX = 'build/'
  }
}