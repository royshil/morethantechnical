/*
 *     Generated by class-dump 3.1.2.
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2007 by Steve Nygard.
 */

/*
 * File: PhotoLibrary.framework/PhotoLibrary
 * Arch: PowerPC 7400 (ppc7400)
 *       Current version: 1.0.0, Compatibility version: 1.0.0
 */

#import "MusicLibrary.h"
#import "TelephonyUI.h"
#import "UIProgressHUD.h"

@class PLImageTile;

@interface PLCameraAlbum : MLPhotoAlbum
{
    MLPhotoDCIMDirectory *_dcimDirectory;
}

+ (id)sharedInstance;
+ (void)clearSharedInstance;
- (id)init;
- (void)dealloc;
- (void)dcimContentsDidChange;
- (id)name;
- (id)images;
- (unsigned int)count;
- (id)posterImage;
- (BOOL)_addImage:(id)fp8 withPreview:(id)fp12 exifProperties:(id)fp16 date:(id)fp20 jpegData:(struct __CFData *)fp24 notifyingTarget:(id)fp28 selector:(SEL)fp32 contextInfo:(void *)fp36 sendPath:(BOOL)fp40;
- (BOOL)addImage:(id)fp8 withPreview:(id)fp12 exifProperties:(id)fp16 date:(id)fp20 jpegData:(struct __CFData *)fp24 notifyingTarget:(id)fp28 selector:(SEL)fp32 contextInfo:(void *)fp36;
- (BOOL)addImage:(id)fp8 withPreview:(id)fp12 exifProperties:(id)fp16 date:(id)fp20 jpegData:(struct __CFData *)fp24 notifyingTargetWithPath:(id)fp28 selector:(SEL)fp32;
- (void)deleteImageAtIndex:(int)fp8;
- (void)cameraImageFinishedSaving:(id)fp8;

@end

@interface PLCameraImage : MLPhoto
{
    MLPhotoDCFFileGroup *_fileGroup;
}

+ (void)setMaxImageSize:(int)fp8;
+ (int)imageIDForFileGroup:(id)fp8;
- (id)initWithDCFFileGroup:(id)fp8;
- (void)dealloc;
- (id)fileGroup;
- (void)fileGroupWillBeginSaving:(id)fp8;
- (void)fileGroupDidFinishSaving:(id)fp8;
- (int)compare:(id)fp8;
- (id)description;
- (id)thumbnailImage;
- (struct CGImage *)_createCGImageForFormat:(int)fp8 orientation:(int *)fp12 ignoringSizeCaps:(BOOL)fp16 properties:(const struct __CFDictionary **)fp20;
- (struct CGImage *)createFullScreenCGImageRef:(int *)fp8 properties:(const struct __CFDictionary **)fp12;
- (struct CGImage *)createFullSizeCGImageRef:(int *)fp8;
- (struct CGImage *)createLowResolutionFullScreenCGImageRef;
- (BOOL)hasFullSizeImage;
- (void)deleteFiles;

@end

@interface MLPhotoDCFFileGroup (CameraAdditions)
- (int)_orientationForExifProperties:(id)fp8;
- (void)writeJPEG:(id)fp8;
- (void)writeJPEGFinished:(id)fp8;
- (void)setImage:(id)fp8 previewImage:(id)fp12 exifProperties:(id)fp16 date:(id)fp20 jpegData:(struct __CFData *)fp24 completionTarget:(id)fp28 completionSelector:(SEL)fp32 contextInfo:(void *)fp36;
@end

@interface TPCameraPushButton : TPPushButton
{
    UIImageView *_cameraIcon;
}

- (void)dealloc;
- (void)setShowsCameraIcon:(BOOL)fp8;
- (BOOL)pointInside:(struct CGPoint)fp8 forEvent:(struct __GSEvent *)fp16;

@end


@interface PLCropLCDLayer : UIView
{
    NSString *_subtitle;
    NSString *_title;
}

- (void)dealloc;
- (void)setTitle:(id)fp8;
- (id)title;
- (void)setSubtitle:(id)fp8;
- (void)drawRect:(struct CGRect)fp8;

@end

@interface PLCropOverlay : UIView
{
    id _delegate;
    PLCropLCDLayer *_lcd;
    UIImageView *_lcdImage;
    TPBottomDualButtonBar *_bottomBar;
    TPPushButton *_cancelButton;
    TPCameraPushButton *_okButton;
    UIProgressHUD *_hud;
}

- (id)initWithSize:(struct CGSize)fp8 setCropTitle:(id)fp16 offsettingStatusBar:(BOOL)fp20;
- (id)initWithSize:(struct CGSize)fp8 setCropTitle:(id)fp16;
- (void)dealloc;
- (void)statusBarHeightDidChange:(id)fp8;
- (void)setDelegate:(id)fp8;
- (void)dismiss;
- (void)_fadeOutCompleted:(id)fp8;
- (void)cancelButtonClicked:(id)fp8;
- (void)okButtonClicked:(id)fp8;
- (void)setEnabled:(BOOL)fp8;
- (id)hitTest:(struct CGPoint)fp8 forEvent:(struct __GSEvent *)fp16;
- (id)hitTest:(struct CGPoint)fp8 withEvent:(id)fp16;
- (struct CGRect)cropRect;
- (struct CGRect)scrollIndicatorRect;
- (void)setTitle:(id)fp8;
- (void)setSubtitle:(id)fp8;
- (void)setOKButtonColor:(int)fp8;
- (void)setOKButtonTitle:(id)fp8;
- (void)setOKButtonShowsCamera:(BOOL)fp8;
- (void)setCancelButtonTitle:(id)fp8;
- (void)setShowProgress:(BOOL)fp8 title:(id)fp12;
- (void)removeProgress;
- (void)setProgressDone;
- (id)dictionaryWithCropResultsFromImageScroller:(id)fp8 withOptions:(int)fp12;

@end


@interface PLPhotosButtonBar : UIToolbar
{
}

+ (id)backgroundImage;
+ (float)defaultHeight;
- (id)initInView:(id)fp8 withItems:(CDAnonymousStruct1 *)fp12 withCount:(int)fp16;
- (void)drawRect:(struct CGRect)fp8;

@end


@interface PLCameraButtonBar : PLPhotosButtonBar
{
    int _centerButtonTag;
}

- (void)positionButtons:(id)fp8 tags:(int *)fp12 count:(int)fp16 group:(int)fp20;
- (void)dealloc;
- (void)start;
- (void)stop;
- (id)cameraButton;
- (void)deviceOrientationChanged;
- (void)setCenterButtonTag:(int)fp8;

@end

@interface PLSilverCameraButtonBar : PLCameraButtonBar
{
}

+ (id)backgroundImage;
- (void)positionButtons:(id)fp8 tags:(int *)fp12 count:(int)fp16 group:(int)fp20;

@end


@interface PLCameraButton : UIThreePartButton
{
    UIView *_rotationHolder;
    UIImageView *_cameraIcon;
    NSMutableArray *_buttonImages;
    BOOL _lockEnabled;
    BOOL _isLandscape;
}

- (id)initWithDefaultSize;
- (BOOL)pointInside:(struct CGPoint)fp8 forEvent:(struct __GSEvent *)fp16;
- (void)dealloc;
- (void)updateButtonBackgrounds;
- (void)setDeviceOrientation:(int)fp8;
- (void)lockEnabled:(BOOL)fp8;
- (void)setEnabled:(BOOL)fp8;
- (void)_setHighlightOnMouseDown:(BOOL)fp8;

@end

@interface PLCameraWindow : UIWindow
{
    BOOL _filterChordChangeEvents;
    UIView *_chordedMouseDownView;
    struct CGPoint _chordedMouseDragPoint;
    int _lastFingerCount;
}

- (id)viewForEvent:(struct __GSEvent *)fp8 newPoint:(struct CGPoint *)fp12;
- (void)sendGSEvent:(struct __GSEvent *)fp8;
- (void)setFilterChordChangeEvents:(BOOL)fp8;

@end

/*
@interface PLCameraView : UIView
{
    UIToolbar *_cameraButtonBar;
    int _availablePictureCount;
    int _captureOrientation;
    int _photoSavingOptions;
    BOOL _manipulatingCrop;
    UIView *_fourByThreeBackstop;
    UIView *_previewView;
    UIView *_animatedCaptureView;
    UIView *_irisView;
    UIView *_staticIrisView;
    int _pictureCapacity;
    BOOL _irisIsClosed;
    PLCropOverlay *_cropOverlay;
    PLImageTile *_imageTile;
    id _delegate;
    int _enabledGestures;
    unsigned int _showsCropOverlay:1;
    unsigned int _allowsEditing:1;
    unsigned int _changesStatusBar:1;
    NSString *_cropTitle;
    NSString *_cropSubtitle;
    NSString *_cropButtonTitle;
}

- (void)_playShutterSound;
- (void)_showCropOverlayWithTitle:(id)fp8 subtitle:(id)fp12 OKButtonTitle:(id)fp16 OKButtonColor:(int)fp20 OKButtonShowCamera:(BOOL)fp24 cancelButtonTitle:(id)fp28;
- (void)_updateStatusBar;
- (void)_addAdditionalEXIFPropertiesForCurrentState:(id)fp8 date:(id)fp12 toDictionary:(id)fp16;
- (int)_sensorOrientationForDeviceOrientation:(int)fp8;
- (id)initWithFrame:(struct CGRect)fp8;
- (void)dealloc;
- (void)setDelegate:(id)fp8;
- (void)setEnabledGestures:(int)fp8;
- (void)setPhotoSavingOptions:(int)fp8;
- (int)photoSavingOptions;
- (void)_updateImageEditability;
- (void)setAllowsImageEditing:(BOOL)fp8;
- (void)setChangesStatusBar:(BOOL)fp8;
- (void)setShowsCropOverlay:(BOOL)fp8;
- (void)setCropTitle:(id)fp8 subtitle:(id)fp12 buttonTitle:(id)fp16;
- (BOOL)shouldShowIris;
- (void)setCameraButtonBar:(id)fp8;
- (id)buttonBar;
- (id)imageTile;
- (void)cameraShutterClicked:(id)fp8;
- (void)viewWillBeDisplayed;
- (void)viewWasDisplayed;
- (void)viewWillBeRemoved;
- (void)cropOverlayWasCancelled:(id)fp8;
- (void)cropOverlayWasOKed:(id)fp8;
- (void)cropOverlay:(id)fp8 didFinishSaving:(id)fp12;
- (BOOL)imageViewIsDisplayingLandscape:(id)fp8;
- (void)willStartGesture:(int)fp8 inView:(id)fp12 forEvent:(struct __GSEvent *)fp16;
- (void)tearDownIris;
- (void)primeStaticClosedIris;
- (void)showStaticClosedIris;
- (void)hideStaticClosedIris;
- (BOOL)irisIsClosed;
- (void)closeIris:(BOOL)fp8 didFinishSelector:(SEL)fp12;
- (void)openIrisWithDidFinishSelector:(SEL)fp8;
- (void)closeOpenIris;
- (void)openIrisAnimationFinished;
- (void)closeIrisAnimationFinished;
- (void)closeOpenIrisAnimationFinished;

@end
*/
@class CALayer;
@class CameraDevice;
@class CameraImageQueueHelper;

@interface PLCameraController : NSObject 
{
    CALayer *_cameraLayer;
    struct CameraDevice { } *_camera;
    struct CameraImageQueueHelper { } *_cameraHelper;
    id _delegate;
    UIView *_previewView;
    BOOL _isPreviewing;
    BOOL _isLocked;
    BOOL _wasPreviewingBeforeDeviceLock;
}

+ (id)sharedInstance;

- (id)init;
- (void)dealloc;
- (void)_setIsReady;
- (BOOL)isReady;
- (void)_applicationSuspended;
- (void)_applicationResumed;
- (void)_tookPicture:(struct __CoreSurfaceBuffer { }*)arg1;
- (void)_tookPicture:(struct CGImage {}*)arg1 jpegData:(struct __CFData { }*)arg2 imageProperties:(struct __CFDictionary { }*)arg3;
- (CameraImageQueueHelper*)_cameraHelper;
- (BOOL)_setupCamera;
- (void)_tearDownCamera;
- (void)setDelegate:(id)arg1;
- (id)delegate;
- (CGRect)_cameraFrame;
- (id)previewView;
- (void)startPreview;
- (void)stopPreview;
- (void)capturePhoto;

@end

@interface PLCameraView : UIView 
{
    UIToolbar *_cameraButtonBar;
    NSInteger _availablePictureCount;
    NSInteger _captureOrientation;
    NSInteger _photoSavingOptions;
    BOOL _manipulatingCrop;
    UIView *_fourByThreeBackstop;
    PLCameraController *_cameraController;
    UIView *_previewView;
    UIView *_animatedCaptureView;
    UIView *_irisView;
    UIView *_staticIrisView;
    NSInteger _pictureCapacity;
    BOOL _irisIsClosed;
    PLCropOverlay *_cropOverlay;
    PLImageTile *_imageTile;
    id _delegate;
    NSInteger _enabledGestures;
    unsigned int _showsCropOverlay : 1;
    unsigned int _allowsEditing : 1;
    unsigned int _changesStatusBar : 1;
    NSString *_cropTitle;
    NSString *_cropSubtitle;
    NSString *_cropButtonTitle;
}


- (void)_playShutterSound;
- (void)_showCropOverlayWithTitle:(id)arg1 subtitle:(id)arg2 OKButtonTitle:(id)arg3 OKButtonColor:(NSInteger)arg4 OKButtonShowCamera:(BOOL)arg5 cancelButtonTitle:(id)arg6;
- (void)_updateStatusBar;
- (void)_addAdditionalEXIFPropertiesForCurrentState:(id)arg1 date:(id)arg2 toDictionary:(id)arg3;
- (NSInteger)_sensorOrientationForDeviceOrientation:(NSInteger)arg1;
- (void)cameraControllerReadyStateChanged:(id)arg1;
- (void)cameraController:(id)arg1 tookPicture:(id)arg2 withPreview:(id)arg3 jpegData:(struct __CFData *)arg4 imageProperties:(id)arg5;
- (void)setupAnimateCameraPreviewDown:(id)arg1;
- (void)animateCameraPreviewDown;
- (void)_storePictureAnimation:(id)arg1 finished:(id)arg2;
- (id)initWithFrame:(CGRect)arg1;
- (void)dealloc;
- (void)setDelegate:(id)arg1;
- (void)setEnabledGestures:(NSInteger)arg1;
- (void)setPhotoSavingOptions:(NSInteger)arg1;
- (NSInteger)photoSavingOptions;
- (void)_updateImageEditability;
- (void)setAllowsImageEditing:(BOOL)arg1;
- (void)setChangesStatusBar:(BOOL)arg1;
- (void)setShowsCropOverlay:(BOOL)arg1;
- (void)setCropTitle:(id)arg1 subtitle:(id)arg2 buttonTitle:(id)arg3;
- (BOOL)shouldShowIris;
- (void)setCameraButtonBar:(id)arg1;
- (id)buttonBar;
- (id)imageTile;
- (void)takePictureOpenIrisAnimationFinished;
- (void)takePictureCloseIrisAnimationFinished;
- (void)cameraShutterClicked:(id)arg1;
- (void)viewWillBeDisplayed;
- (void)viewWasDisplayed;
- (void)viewWillBeRemoved;
- (void)cropOverlayWasCancelled:(id)arg1;
- (void)cropOverlayWasOKed:(id)arg1;
- (void)cropOverlay:(id)arg1 didFinishSaving:(id)arg2;
- (BOOL)imageViewIsDisplayingLandscape:(id)arg1;
- (void)willStartGesture:(NSInteger)arg1 inView:(id)arg2 forEvent:(struct __GSEvent { }*)arg3;
- (void)tearDownIris;
- (void)primeStaticClosedIris;
- (void)showStaticClosedIris;
- (void)hideStaticClosedIris;
- (BOOL)irisIsClosed;
- (void)closeIris:(BOOL)arg1 didFinishSelector:(SEL)arg2;
- (void)openIrisWithDidFinishSelector:(SEL)arg1;
- (void)closeOpenIris;
- (void)openIrisAnimationFinished;
- (void)closeIrisAnimationFinished;
- (void)closeOpenIrisAnimationFinished;

@end


@interface PLCameraViewController : UIViewController
{
    PLCameraView *_cameraView;
}

- (BOOL)_displaysFullScreen;
- (void)dealloc;
- (void)loadView;
- (void)setParentViewController:(id)fp8;
- (void)viewWillAppear:(BOOL)fp8;
- (void)viewDidAppear:(BOOL)fp8;
- (void)viewWillDisappear:(BOOL)fp8;
- (void)setAllowsEditing:(BOOL)fp8;
- (void)_editabilityChanged:(id)fp8;
- (void)cameraViewCancelled:(id)fp8;
- (void)cameraView:(id)fp8 photoSaved:(id)fp12;
- (BOOL)cameraViewShouldShowIris:(id)fp8;
- (BOOL)cameraViewShouldShowPreviewAfterSelection:(id)fp8;
- (BOOL)cameraViewShouldShowProgressWhileCapturing:(id)fp8;

@end

