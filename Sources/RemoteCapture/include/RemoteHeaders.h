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
//   unsigned long _precision;
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
//   bool __expectedToBecomeDrag;
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
@property () long type; // Tq,N,S_setType:,V_type
@property () unsigned long _senderID; // TQ,N,S_setSenderID:,V_senderID
@property () __IOHIDEvent * _hidEvent; // T^{__IOHIDEvent=},N,S_setHidEvent:,V_hidEvent
@property () double initialTouchTimestamp; // Td,N,V_initialTouchTimestamp
@property () double timestamp; // Td,N,V_timestamp
@property () long phase; // Tq,N
@property () unsigned long tapCount; // TQ,N
@property () UIWindow * window; // T@"UIWindow",&,N
@property () UIView * view; // T@"UIView",&,N
@property () unsigned char _forceStage; // TC,N,S_setForceStage:,V_forceStage
@property () bool _isPointerTouch; // TB,N,S_setIsPointerTouch:
@property () bool _isRestingTouch; // TB,N,S_setIsRestingTouch:
@property () bool _isTapToClick; // TB,N,S_setIsTapToClick:
@property () double majorRadius; // Td,R,N
@property () double majorRadiusTolerance; // Td,R,N
@property () NSArray * gestureRecognizers; // T@"NSArray",R,C,N
@property () double force; // Td,R,N
@property () double maximumPossibleForce; // Td,R,N
@property () double altitudeAngle; // Td,R,N
@property () NSNumber * estimationUpdateIndex; // T@"NSNumber",R,N
@property () long estimatedProperties; // Tq,R,N
@property () long estimatedPropertiesExpectingUpdates; // Tq,R,N
@property () long _forwardablePhase; // Tq,N,S_setForwardablePhase:
@property () UIResponder * _responder; // T@"UIResponder",&,N,S_setResponder:
@property () unsigned long hash; // TQ,R
@property () Class superclass; // T#,R
@property () NSString * description; // T@"NSString",R,C
@property () NSString * debugDescription; // T@"NSString",R,C

+ (id)_createTouchesWithGSEvent:(__GSEvent *)a0 phase:(long)a1 view:(id)a2 ;

- (void)dealloc ;
- (id)description ;
//- (void).cxx_destruct ;
- (long)type ;
- (double)timestamp ;
- (void)setTimestamp:(double)a0 ;
- (long)info ;
- (UIView *)view ;
- (void)setView:(UIView *)a0 ;
- (UIWindow *)window ;
- (long)phase ;
- (__IOHIDEvent *)_hidEvent ;
- (id)gestureRecognizers ;
- (CGPoint)locationInView:(id)a0 ;
- (id)_clone ;
- (id)_gestureRecognizers ;
- (CGPoint)previousLocationInView:(id)a0 ;
- (unsigned long)_senderID ;
- (bool)_isPointerTouch ;
- (void)setWindow:(UIWindow *)a0 ;
- (unsigned long)tapCount ;
- (unsigned)_touchIdentifier ;
- (double)majorRadius ;
- (double)force ;
- (id)_responder ;
- (void)setPhase:(long)a0 ;
- (CGPoint)_locationInSceneReferenceSpace ;
- (void)_loadStateFromTouch:(id)a0 ;
- (id)_phaseDescription ;
- (void)setIsDelayed:(bool)a0 ;
- (bool)isDelayed ;
- (long)_compareIndex:(id)a0 ;
- (unsigned long)_edgeAim ;
- (bool)_edgeForceActive ;
- (long)_edgeType ;
- (bool)_edgeForcePending ;
- (CGPoint)_previousLocationInSceneReferenceSpace ;
- (void)_setLocationInWindow:(CGPoint)a0 resetPrevious:(bool)a1 ;
- (void)setTapCount:(unsigned long)a0 ;
- (long)_pathIndex ;
- (unsigned char)_pathIdentity ;
- (float)_pathMajorRadius ;
- (void)_setType:(long)a0 ;
- (float)_zGradient ;
- (double)maximumPossibleForce ;
- (void)_setForwardablePhase:(long)a0 ;
- (long)_responderForwardableType ;
- (void)_setResponder:(id)a0 ;
- (id)_forwardingRecord ;
- (SEL)_responderSelectorForPhase:(long)a0 ;
- (bool)_wantsForwardingFromResponder:(id)a0 toNextResponder:(id)a1 withEvent:(id)a2 ;
- (double)initialTouchTimestamp ;
- (void)_clearForReenteringHoverInWindow:(id)a0 ;
- (void)_setPreviousTouch:(id)a0 ;
- (bool)_isAbandoningForwardingRecord ;
- (void)_abandonForwardingRecord ;
- (id)_mutableForwardingRecord ;
- (long)_forwardablePhase ;
- (void)_clonePropertiesToTouch:(id)a0 ;
- (void)_setHidEvent:(__IOHIDEvent *)a0 ;
- (CGVector)azimuthUnitVectorInView:(id)a0 ;
- (double)_pressure ;
- (CGPoint)preciseLocationInView:(id)a0 ;
- (CGPoint)precisePreviousLocationInView:(id)a0 ;
- (double)azimuthAngleInView:(id)a0 ;
- (double)altitudeAngle ;
- (id)estimationUpdateIndex ;
- (long)estimatedProperties ;
- (long)estimatedPropertiesExpectingUpdates ;
- (void)_clearGestureRecognizers ;
- (void)_setIsPointerTouch:(bool)a0 ;
- (bool)_isRestingTouch ;
- (void)_setIsRestingTouch:(bool)a0 ;
- (bool)_isTapToClick ;
- (void)_setIsTapToClick:(bool)a0 ;
- (void)_setPathIndex:(long)a0 ;
- (void)_setPathIdentity:(unsigned char)a0 ;
- (void)_setEdgeType:(long)a0 ;
- (void)_setEdgeAim:(unsigned long)a0 ;
- (double)majorRadiusTolerance ;
- (void)_setZGradient:(float)a0 ;
- (void)_setSenderID:(unsigned long)a0 ;
- (unsigned char)_forceStage ;
- (void)_setForceStage:(unsigned char)a0 ;
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

- (void)dealloc ;
- (id)description ;
//- (void).cxx_destruct ;
- (long)type ;
- (id)_init ;
- (void)_setHIDEvent:(__IOHIDEvent *)a0 ;
- (id)_windows ;
- (id)_initWithEvent:(__GSEvent *)a0 touches:(id)a1 ;
- (id)_gestureRecognizersForWindow:(id)a0 ;
- (bool)_sendEventToGestureRecognizer:(id)a0 ;
- (unsigned long)_inputPrecision ;
- (id)_firstTouchForView:(id)a0 ;
- (bool)_containsHIDPointerEvent ;
- (id)touchesForWindow:(id)a0 ;
- (id)allTouches ;
- (CGPoint)_digitizerLocation ;
- (id)touchesForGestureRecognizer:(id)a0 ;
- (id)_touchesForWindow:(id)a0 ;
- (id)_cloneEvent ;
- (void)_gestureRecognizerNoLongerNeedsSendEvent:(id)a0 ;
- (id)_viewsForWindow:(id)a0 ;
- (id)_touchesForView:(id)a0 withPhase:(long)a1 ;
- (id)_exclusiveTouchWindows ;
- (void)_addGestureRecognizersForTouchContinuationInView:(id)a0 toTouch:(id)a1 ;
- (void)_addTouch:(id)a0 forDelayedDelivery:(bool)a1 ;
- (id)_touchesForKey:(id)a0 ;
- (void)_invalidateGestureRecognizerForWindowCache ;
- (void)_removeTouch:(id)a0 fromGestureRecognizer:(id)a1 ;
- (id)_touchesForGesture:(id)a0 withPhase:(long)a1 ;
- (double)_initialTouchTimestamp ;
- (void)_windowNoLongerAwaitingSystemGestureNotification:(id)a0 ;
- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 ;
- (id)touchesForView:(id)a0 ;
- (id)coalescedTouchesForTouch:(id)a0 ;
- (double)_initialTouchTimestampForWindow:(id)a0 ;
- (void)_clearViewForTouch:(id)a0 ;
- (void)_removeTouch:(id)a0 ;
- (void)_collectGestureRecognizersForView:(id)a0 withBlock:(id)a1 ;
- (void)_addGestureRecognizersForView:(id)a0 toTouch:(id)a1 forContinuation:(bool)a2 ;
- (void)_addGestureRecognizersForView:(id)a0 toTouch:(id)a1 ;
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
- (id)_windowsAwaitingLatentSystemGestureNotificationDeliveredToEventWindow:(id)a0 ;
- (void)_addCoalescedTouch:(id)a0 forIndex:(long)a1 hidEvent:(__IOHIDEvent *)a2 ;
- (void)_moveCoalescedTouchesFromHidEvent:(__IOHIDEvent *)a0 toHidEvent:(__IOHIDEvent *)a1 ;
- (id)_lastPendingCoalescedTouchForIndex:(long)a0 hidEvent:(__IOHIDEvent *)a1 ;
- (id)predictedTouchesForTouch:(id)a0 ;
- (long)singleAllowableExternalTouchPathIndex ;
- (void)setSingleAllowableExternalTouchPathIndex:(long)a0 ;

@end
#endif
