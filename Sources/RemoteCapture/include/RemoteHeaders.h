//
//  RemoteHeaders.h
//  Remote
//
//  $Id: //depot/Remote/Sources/RemoteCapture/include/RemoteCapture.h#11 $
//

#import <Foundation/Foundation.h>
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#import <UIKit/UIKit.h>

#define __CFDictionary NSDictionary
typedef CFTypeRef __IOHIDEvent;
typedef CFTypeRef __GSEvent;

@interface UITouch (FromXprobe) // : NSObject <_UIResponderForwardable> {
//   double _movementMagnitudeSquared;
//   long _phase;
//   unsigned long _tapCount;
//   long _edgeType;
//   unsigned long _edgeAim;
//   unsigned _touchIdentifier;
//   UIWindow *_window;
//   UIView *_view;
//   UIView *_warpedIntoView;
//   NSMutableArray *_gestureRecognizers;
//   NSMutableArray *_forwardingRecord;
//   CGPoint _locationInWindow;
//   CGPoint _previousLocationInWindow;
//   CGPoint _preciseLocationInWindow;
//   CGPoint _precisePreviousLocationInWindow;
//   double _previousPressure;
//   long _pathIndex;
//   unsigned char _pathIdentity;
//   double _pathMajorRadius;
//   double _majorRadiusTolerance;
//   double _pressure;
//   double _maxObservedPressure;
//   float _zGradient;
//   ?="_firstTouchForView"b1"_isTap"b1"_isDelayed"b1"_sentTouchesEnded"b1"_abandonForwardingRecord"b1"_deliversUpdatesInTouchesMovedIsValid"b1"_deliversUpdatesInTouchesMoved"b1"_isPredictedTouch"b1"_didDispatchAsEnded"b1"_isPointerTouch"b1"_isRestingTouch"b1"_isTapToClick"b1} _touchFlags;
//   _UITouchPredictor *_touchPredictor;
//   bool _eaten;
//   bool _needsForceUpdate;
//   bool _hasForceUpdate;
//   unsigned char _forceStage;
//   double _timestamp;
//   long _forceCorrelationToken;
//   double _maximumPossiblePressure;
//   unsigned long _senderID;
//   __IOHIDEvent *_hidEvent;
//   double _altitudeAngle;
//   long _type;
//   id<_UITouchPhaseChangeDelegate> __phaseChangeDelegate;
//   UIWindow *__windowServerHitTestWindow;
//   double _azimuthAngleInCADisplay;
//   double _azimuthAngleInWindow;
//   double _initialTouchTimestamp;
//   CGSize _displacement;
//}

@property () long _pathIndex; // Tq,N,S_setPathIndex:,V_pathIndex
@property () unsigned char _pathIdentity; // TC,N,S_setPathIdentity:,V_pathIdentity
@property () double _pressure; // Td,R,N,V_pressure
@property () float _zGradient; // Tf,N,S_setZGradient:,V_zGradient
@property () long _edgeType; // Tq,N,S_setEdgeType:,V_edgeType
@property () unsigned long _edgeAim; // TQ,N,S_setEdgeAim:,V_edgeAim
@property () bool _eaten; // TB,N,G_isEaten,S_setEaten:,V_eaten
@property () CGSize _displacement; // T{CGSize=dd},N,S_setDisplacement:,V_displacement
@property () id/* <_UITouchPhaseChangeDelegate> */ _phaseChangeDelegate; // T@"<_UITouchPhaseChangeDelegate>",&,N,S_setPhaseChangeDelegate:,V__phaseChangeDelegate
@property () UIWindow * _windowServerHitTestWindow; // T@"UIWindow",&,N,S_setWindowServerHitTestWindow:,V__windowServerHitTestWindow
@property () long type; // Tq,N,S_setType:,V_type
@property () double azimuthAngle; // Td,R,N
@property () double azimuthAngleInCADisplay; // Td,N,S_setAzimuthAngleInCADisplay:,V_azimuthAngleInCADisplay
@property () double azimuthAngleInWindow; // Td,R,N,V_azimuthAngleInWindow
@property () double altitudeAngle; // Td,N,S_setAltitudeAngle:,V_altitudeAngle
@property () bool _isPredictedTouch; // TB,N,S_setIsPredictedTouch:
@property () bool _needsForceUpdate; // TB,N,S_setNeedsForceUpdate:,V_needsForceUpdate
@property () bool _hasForceUpdate; // TB,N,S_setHasForceUpdate:,V_hasForceUpdate
@property () long _forceCorrelationToken; // Tq,N,S_setForceCorrelationToken:,V_forceCorrelationToken
@property () double _maximumPossiblePressure; // Td,N,S_setMaximumPossiblePressure:,V_maximumPossiblePressure
@property () double _unclampedForce; // Td,R,N
@property () unsigned long _senderID; // TQ,N,S_setSenderID:,V_senderID
@property () __IOHIDEvent * _hidEvent; // T^{__IOHIDEvent=},N,S_setHidEvent:,V_hidEvent
@property () double initialTouchTimestamp; // Td,N,V_initialTouchTimestamp
@property () double timestamp; // Td,N,V_timestamp
@property () long phase; // Tq,N
@property () unsigned long tapCount; // TQ,N
@property () bool isTap; // TB,N
@property () bool sentTouchesEnded; // TB,N
@property () double majorRadius; // Td,N,V_pathMajorRadius
@property () double majorRadiusTolerance; // Td,N,V_majorRadiusTolerance
@property () UIWindow * window; // T@"UIWindow",&,N
@property () UIView * view; // T@"UIView",&,N
@property () UIView * warpedIntoView; // T@"UIView",&,N
@property () bool _respectsCharge; // TB,R,N
@property () unsigned char _forceStage; // TC,N,S_setForceStage:,V_forceStage
@property () bool _isPointerTouch; // TB,N,S_setIsPointerTouch:
@property () bool _isRestingTouch; // TB,N,S_setIsRestingTouch:
@property () bool _isTapToClick; // TB,N,S_setIsTapToClick:
@property () NSArray * gestureRecognizers; // T@"NSArray",R,C,N
@property () double force; // Td,R,N
@property () double maximumPossibleForce; // Td,R,N
@property () NSNumber * estimationUpdateIndex; // T@"NSNumber",R,N
@property () long estimatedProperties; // Tq,R,N
@property () long estimatedPropertiesExpectingUpdates; // Tq,R,N
@property () unsigned long hash; // TQ,R
@property () Class superclass; // T#,R
@property () NSString * description; // T@"NSString",R,C
@property () NSString * debugDescription; // T@"NSString",R,C
@property () long _forwardablePhase; // Tq,N,S_setForwardablePhase:
@property () UIResponder * _responder; // T@"UIResponder",&,N,S_setResponder:

+ (id)_createTouchesWithGSEvent:(__GSEvent *)a0 phase:(long)a1 view:(id)a2 ;

- (CGPoint)locationInNode:(id)a0 ;
- (CGPoint)previousLocationInNode:(id)a0 ;
- (id)description ;
//- (void).cxx_destruct ;
- (void)dealloc ;
- (UIWindow *)window ;
- (CGPoint)locationInView:(id)a0 ;
- (UIView *)view ;
- (id)gestureRecognizers ;
- (double)timestamp ;
- (void)setView:(UIView *)a0 ;
- (long)type ;
- (void)setPhase:(long)a0 ;
- (void)setTimestamp:(double)a0 ;
- (long)phase ;
- (void)_setLocationInWindow:(CGPoint)a0 resetPrevious:(bool)a1 ;
- (bool)_isPointerTouch ;
- (CGPoint)previousLocationInView:(id)a0 ;
- (void)setWindow:(UIWindow *)a0 ;
- (CGPoint)precisePreviousLocationInView:(id)a0 ;
- (CGPoint)preciseLocationInView:(id)a0 ;
- (double)force ;
- (unsigned long)_senderID ;
- (long)info ;
- (CGSize)_displacement ;
- (void)_setType:(long)a0 ;
- (void)_clearGestureRecognizers ;
- (unsigned long)tapCount ;
- (void)setTapCount:(unsigned long)a0 ;
- (double)maximumPossibleForce ;
- (double)_pressure ;
- (__IOHIDEvent *)_hidEvent ;
- (double)azimuthAngleInView:(id)a0 ;
- (double)altitudeAngle ;
- (id)estimationUpdateIndex ;
- (long)estimatedProperties ;
- (id)_gestureRecognizers ;
- (double)majorRadius ;
- (long)_pathIndex ;
- (unsigned char)_pathIdentity ;
- (float)_pathMajorRadius ;
- (bool)isDelayed ;
- (unsigned)_touchIdentifier ;
- (double)initialTouchTimestamp ;
- (void)_setSenderID:(unsigned long)a0 ;
- (id)_phaseDescription ;
- (id)_responder ;
- (long)_edgeType ;
- (void)setIsDelayed:(bool)a0 ;
- (CGPoint)_locationInSceneReferenceSpace ;
- (CGPoint)_previousLocationInSceneReferenceSpace ;
- (bool)_isRestingTouch ;
- (void)_removeGestureRecognizer:(id)a0 ;
- (id)_clone ;
- (id)_windowServerHitTestWindow ;
- (void)_loadStateFromTouch:(id)a0 ;
- (void)setSentTouchesEnded:(bool)a0 ;
- (id)_rehitTest ;
- (CGPoint)_locationInWindow:(id)a0 ;
- (void)_addGestureRecognizer:(id)a0 ;
- (long)_compareIndex:(id)a0 ;
- (void)setMajorRadius:(double)a0 ;
- (bool)_isStationaryRelativeToTouches:(id)a0 ;
- (unsigned long)_edgeAim ;
- (bool)_edgeForceActive ;
- (bool)_edgeForcePending ;
- (bool)_mightBeConsideredForForceSystemGesture ;
- (void)_setPhaseChangeDelegate:(id)a0 ;
- (float)_zGradient ;
- (double)_unclampedForce ;
- (double)_maximumPossiblePressure ;
- (void)_setEaten:(bool)a0 ;
- (bool)_isEaten ;
- (void)_setForwardablePhase:(long)a0 ;
- (void)_setResponder:(id)a0 ;
- (bool)sentTouchesEnded ;
- (id)_forwardingRecord ;
- (SEL)_responderSelectorForPhase:(long)a0 ;
- (bool)_wantsForwardingFromResponder:(id)a0 toNextResponder:(id)a1 withEvent:(id)a2 ;
- (id)_rehitTestWithEvent:(id)a0 constrainingToCurrentWindow:(bool)a1 ;
- (void)_clearForReenteringHoverInWindow:(id)a0 ;
- (bool)_shouldDeliverTouchForTouchesMoved ;
- (bool)_isFirstTouchForView ;
- (void)_setPreviousTouch:(id)a0 ;
- (id)_predictedTouchesWithEvent:(id)a0 ;
- (bool)_isAbandoningForwardingRecord ;
- (void)_abandonForwardingRecord ;
- (id)_mutableForwardingRecord ;
- (long)_forwardablePhase ;
- (void)_clonePropertiesToTouch:(id)a0 ;
- (void)_setHidEvent:(__IOHIDEvent *)a0 ;
- (void)_updateMovementMagnitudeFromLocation:(CGPoint)a0 toLocation:(CGPoint)a1 ;
- (double)azimuthAngleInCADisplay ;
- (void)_computeAzimuthAngleInWindow ;
- (CGVector)azimuthUnitVectorInView:(id)a0 ;
- (bool)_supportsForce ;
- (double)_standardForceAmount ;
- (void)_updatePredictionsWithCoalescedTouches:(id)a0 ;
- (void)_setIsFirstTouchForView:(bool)a0 ;
- (id)_phaseChangeDelegate ;
- (void)_setLocation:(CGPoint)a0 preciseLocation:(CGPoint)a1 inWindowResetPreviousLocation:(bool)a2 ;
- (CGPoint)_previousLocationInWindow:(id)a0 ;
- (void)_setAltitudeAngle:(double)a0 ;
- (void)_setAzimuthAngleInCADisplay:(double)a0 ;
- (bool)isTap ;
- (void)_setIsPredictedTouch:(bool)a0 ;
- (bool)_isPredictedTouch ;
- (double)azimuthAngle ;
- (long)estimatedPropertiesExpectingUpdates ;
- (bool)_currentlyPredictingTouches ;
- (id)_touchPredictor ;
- (void)_setTouchIdentifier:(unsigned)a0 ;
- (void)setIsTap:(bool)a0 ;
- (void)setMajorRadiusTolerance:(double)a0 ;
- (void)_setPressure:(double)a0 resetPrevious:(bool)a1 ;
- (void)_updateWithChildEvent:(__IOHIDEvent *)a0 ;
- (UIView *)warpedIntoView ;
- (void)setWarpedIntoView:(UIView *)a0 ;
- (bool)_respectsCharge ;
- (void)_updateMovementMagnitudeForLocation:(CGPoint)a0 ;
- (void)_willBeDispatchedAsEnded ;
- (void)_setIsPointerTouch:(bool)a0 ;
- (void)_setIsRestingTouch:(bool)a0 ;
- (bool)_isTapToClick ;
- (void)_setIsTapToClick:(bool)a0 ;
- (void)_setPathIndex:(long)a0 ;
- (void)_setPathIdentity:(unsigned char)a0 ;
- (void)_setDisplacement:(CGSize)a0 ;
- (void)_setEdgeType:(long)a0 ;
- (void)_setEdgeAim:(unsigned long)a0 ;
- (double)majorRadiusTolerance ;
- (void)_setZGradient:(float)a0 ;
- (long)_forceCorrelationToken ;
- (void)_setForceCorrelationToken:(long)a0 ;
- (bool)_needsForceUpdate ;
- (void)_setNeedsForceUpdate:(bool)a0 ;
- (bool)_hasForceUpdate ;
- (void)_setHasForceUpdate:(bool)a0 ;
- (void)_setMaximumPossiblePressure:(double)a0 ;
- (unsigned char)_forceStage ;
- (void)_setForceStage:(unsigned char)a0 ;
- (void)_setWindowServerHitTestWindow:(id)a0 ;
- (double)azimuthAngleInWindow ;
- (void)setInitialTouchTimestamp:(double)a0 ;

@end

@interface UITouchesEvent : UIEvent {
   NSMutableSet *_allTouchesMutable;
   NSSet *_allTouchesImmutableCached;
   __CFDictionary *_keyedTouches;
   __CFDictionary *_keyedTouchesByWindow;
   __CFDictionary *_gestureRecognizersByWindow;
   NSMapTable *_latentSystemGestureWindows;
   __CFDictionary *_coalescedTouches;
   __CFDictionary *_finalTouches;
   bool _isCallingEventObservers;
   NSMutableSet *_exclusiveTouchWindows;
   long _singleAllowableExternalTouchPathIndex;
}

@property () double _initialTouchTimestamp; // Td,R,N
@property () long singleAllowableExternalTouchPathIndex; // Tq,N,V_singleAllowableExternalTouchPathIndex
@property () bool containsHIDPointerEvent; // TB,R,N,G_containsHIDPointerEvent

- (id)description ;
//- (void).cxx_destruct ;
- (void)dealloc ;
- (long)type ;
- (void)_setHIDEvent:(__IOHIDEvent *)a0 ;
- (id)allTouches ;
- (id)_init ;
- (id)touchesForGestureRecognizer:(id)a0 ;
- (id)touchesForWindow:(id)a0 ;
- (id)coalescedTouchesForTouch:(id)a0 ;
- (id)predictedTouchesForTouch:(id)a0 ;
- (id)_cloneEvent ;
- (id)touchesForView:(id)a0 ;
- (double)_initialTouchTimestamp ;
- (id)_initWithEvent:(__GSEvent *)a0 touches:(id)a1 ;
- (id)_windows ;
- (id)_gestureRecognizersForWindow:(id)a0 ;
- (bool)_sendEventToGestureRecognizer:(id)a0 ;
- (id)_firstTouchForView:(id)a0 ;
- (CGPoint)_digitizerLocation ;
- (void)_gestureRecognizerNoLongerNeedsSendEvent:(id)a0 ;
- (id)_viewsForWindow:(id)a0 ;
- (id)_touchesForView:(id)a0 withPhase:(long)a1 ;
- (id)_exclusiveTouchWindows ;
- (void)_addGestureRecognizersForView:(id)a0 toTouch:(id)a1 ;
- (void)_addTouch:(id)a0 forDelayedDelivery:(bool)a1 ;
- (id)_touchesForKey:(id)a0 ;
- (void)_invalidateGestureRecognizerForWindowCache ;
- (void)_removeTouch:(id)a0 fromGestureRecognizer:(id)a1 ;
- (id)_touchesForGesture:(id)a0 withPhase:(long)a1 ;
- (void)_windowNoLongerAwaitingSystemGestureNotification:(id)a0 ;
- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 ;
- (void)_clearViewForTouch:(id)a0 ;
- (void)_removeTouch:(id)a0 ;
- (id)_touchesForWindow:(id)a0 ;
- (void)_collectGestureRecognizersForView:(id)a0 withBlock:(id)a1 ;
- (void)_removeTouchesForKey:(id)a0 ;
- (void)_removeTouchesForWindow:(id)a0 ;
- (void)_touchesForGesture:(id)a0 withPhase:(long)a1 intoSet:(id)a2 ;
- (__CFDictionary *)_coalescedTouchesForHidEvent:(__IOHIDEvent *)a0 ;
- (id)_rawCoalescedTouchesForTouch:(id)a0 ;
- (id)_allTouches ;
- (id)_touchesForGestureRecognizer:(id)a0 ;
- (bool)_anyInterestedGestureRecognizersForTouchInView:(id)a0 ;
- (void)_clearTouches ;
- (void)_moveTouchesFromView:(id)a0 toView:(id)a1 ;
- (double)_initialTouchTimestampForWindow:(id)a0 ;
- (id)_windowsAwaitingLatentSystemGestureNotificationDeliveredToEventWindow:(id)a0 ;
- (void)_addCoalescedTouch:(id)a0 forIndex:(long)a1 hidEvent:(__IOHIDEvent *)a2 ;
- (void)_moveCoalescedTouchesFromHidEvent:(__IOHIDEvent *)a0 toHidEvent:(__IOHIDEvent *)a1 ;
- (id)_lastPendingCoalescedTouchForIndex:(long)a0 hidEvent:(__IOHIDEvent *)a1 ;
- (bool)_containsHIDPointerEvent ;
- (long)singleAllowableExternalTouchPathIndex ;
- (void)setSingleAllowableExternalTouchPathIndex:(long)a0 ;

@end
#endif
