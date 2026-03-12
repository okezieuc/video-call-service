#include "MainWindow.h"
#include <gtest/gtest.h>

class MockMainWindow : public MainWindow {
public:
  int fakeVideoInputCount = 1;
  Qt::PermissionStatus fakeCameraPermissionStatus =
      Qt::PermissionStatus::Undetermined;
  bool requestCameraPermissionCalled = false;
  bool isCameraStarted = false;

  auto getCameraStatus() { return cameraStatus; }

protected:
  Qt::PermissionStatus checkCameraPermissionStatus() const override {
    return fakeCameraPermissionStatus;
  }
  void requestCameraPermission() override {
    requestCameraPermissionCalled = true;
  }
  void startCamera() override {
    isCameraStarted = true;
  }
  int videoInputCount() const override { return fakeVideoInputCount; }
};

class MainWindowTest : public testing::Test {
protected:
  MainWindowTest() {}

  MockMainWindow window;
};

TEST_F(MainWindowTest, UpdateStatus_WithNoVideoInput_SetsDeviceHasCameraFalse) {
  window.fakeVideoInputCount = 0;
  window.updateCameraStatus();
  EXPECT_EQ(window.getCameraStatus().deviceHasCamera, Tristate::False);
}

TEST_F(MainWindowTest,
       UpdateStatus_UpdatesCameraStatus) {
  window.fakeCameraPermissionStatus = Qt::PermissionStatus::Granted;
  window.fakeVideoInputCount = 1;
  window.updateCameraStatus();
  EXPECT_EQ(window.getCameraStatus().deviceHasCamera, Tristate::True);
  EXPECT_EQ(window.getCameraStatus().isPermissionGranted, Tristate::True);
  EXPECT_TRUE(window.isCameraStarted);
}


TEST_F(MainWindowTest,
       UpdateStatus_WithNonZeroVideoInput_SetsDeviceHasCameraTrue) {
  window.fakeVideoInputCount = 3;
  window.updateCameraStatus();
  EXPECT_EQ(window.getCameraStatus().deviceHasCamera, Tristate::True);
}

TEST_F(MainWindowTest,
       UpdateStatus_WhenPermissionIsUndetermined_RequestsCameraPermission) {
  window.fakeCameraPermissionStatus = Qt::PermissionStatus::Undetermined;
  window.updateCameraStatus();
  EXPECT_TRUE(window.requestCameraPermissionCalled);
}

TEST_F(MainWindowTest,
       UpdateStatus_WhenPermissionIsDetermined_WontRequestCameraPermission) {
  window.fakeCameraPermissionStatus = Qt::PermissionStatus::Granted;
  window.updateCameraStatus();
  EXPECT_FALSE(window.requestCameraPermissionCalled);
}
