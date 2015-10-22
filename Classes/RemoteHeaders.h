//
//  RemoteHeaders.h
//  Remote
//

#import <UIKit/UIKit.h>

typedef CFTypeRef __IOHIDEvent;

@interface UITouch ()

- (void)dealloc ;
- (id)description ;
//- (CGPoint)locationInView:(id)a0 ;
- (void)setWindow:(id)a0 ;
- (__IOHIDEvent *)_hidEvent ;
- (void)setIsTap:(BOOL)a0 ;
//- (long)phase ;
- (BOOL)isTap ;
- (void)setTapCount:(unsigned long)a0 ;
- (BOOL)_isEaten ;
- (void)setPhase:(long)a0 ;
- (void)_setEaten:(BOOL)a0 ;
//- (CGFloat)force ;
//- (id)gestureRecognizers ;
- (void)_setForwardablePhase:(long)a0 ;
- (id)_responder ;
- (void)_setResponder:(id)a0 ;
- (BOOL)sentTouchesEnded ;
- (void)_clearGestureRecognizers ;
- (void)_addGestureRecognizer:(id)a0 ;
- (void)setView:(id)a0 ;
- (id)_clone ;
- (void)_setPreviousTouch:(id)a0 ;
- (CGFloat)_pressure ;
- (void)_setLocationInWindow:(CGPoint)a0 resetPrevious:(BOOL)a1 ;
- (void)_setPressure:(CGFloat)a0 resetPrevious:(BOOL)a1 ;
//- (void)_setHidEvent:(__IOHIDEvent *)a0 ;
- (void)_setPathIndex:(unsigned char)a0 ;
- (void)_setPathIdentity:(unsigned char)a0 ;
- (void)setMajorRadius:(CGFloat)a0 ;
- (void)setMajorRadiusTolerance:(CGFloat)a0 ;
- (void)_setType:(long)a0 ;
- (void)_setSenderID:(unsigned long)a0 ;
- (void)_setMaximumPossiblePressure:(CGFloat)a0 ;
- (id)_windowServerHitTestWindow ;
- (CGSize)_displacement ;
- (unsigned char)_pathIndex ;
- (void)_setDisplacement:(CGSize)a0 ;
- (void)_setWindowServerHitTestWindow:(id)a0 ;
- (void)_setEdgeType:(long)a0 ;
- (id)warpedIntoView ;
- (void)setWarpedIntoView:(id)a0 ;
- (void)_setIsFirstTouchForView:(BOOL)a0 ;
- (void)_updateMovementMagnitudeForLocation:(CGPoint)a0 ;
- (unsigned long)_senderID ;
- (void)setSentTouchesEnded:(BOOL)a0 ;
- (long)_edgeType ;
- (id)_gestureRecognizers ;
- (BOOL)isDelayed ;
//- (CGPoint)previousLocationInView:(id)a0 ;
//- (unsigned long)tapCount ;
- (id)_forwardingRecord ;
- (SEL)_responderSelectorForPhase:(long)a0 ;
- (BOOL)_wantsForwardingFromResponder:(id)a0 toNextResponder:(id)a1 withEvent:(id)a2 ;
- (float)_pathMajorRadius ;
- (CGPoint)_locationInSceneReferenceSpace ;
- (void)_removeGestureRecognizer:(id)a0 ;
- (BOOL)_isFirstTouchForView ;
- (long)_forwardablePhase ;
- (BOOL)_isAbandoningForwardingRecord ;
- (id)_mutableForwardingRecord ;
- (void)_abandonForwardingRecord ;
- (void)_clonePropertiesToTouch:(id)a0 ;
- (void)_updateMovementMagnitudeFromLocation:(CGPoint)a0 toLocation:(CGPoint)a1 ;
//- (CGFloat)majorRadius ;
- (CGFloat)_unclampedForce ;
//- (CGFloat)maximumPossibleForce ;
- (BOOL)_supportsForce ;
- (CGFloat)_maximumPossiblePressure ;
- (CGFloat)_standardForceAmount ;
- (void)setIsDelayed:(BOOL)a0 ;
- (void)_loadStateFromTouch:(id)a0 ;
- (BOOL)_edgeForcePending ;
- (BOOL)_edgeForceActive ;
- (unsigned char)_pathIdentity ;
//- (CGFloat)majorRadiusTolerance ;
- (long)_forceCorrelationToken ;
- (void)_setForceCorrelationToken:(long)a0 ;
- (BOOL)_needsForceUpdate ;
- (void)_setNeedsForceUpdate:(BOOL)a0 ;
- (id)_phaseChangeDelegate ;
- (void)_setPhaseChangeDelegate:(id)a0 ;
- (id)_phaseDescription ;
- (CGPoint)_locationInWindow:(id)a0 ;
- (CGPoint)_previousLocationInWindow:(id)a0 ;
- (CGPoint)_previousLocationInSceneReferenceSpace ;
- (long)_compareIndex:(id)a0 ;
- (BOOL)_isStationaryRelativeToTouches:(id)a0 ;
- (CGFloat)_distanceFrom:(id)a0 inView:(id)a1 ;
//- (long)type ;
//- (id)window ;
- (long)info ;
//- (CGFloat)timestamp ;
- (void)setTimestamp:(CGFloat)a0 ;
//- (id)view ;

@end

@class __GSEvent;
@class __CFDictionary;

@interface UITouchesEvent : UIEvent

- (void)dealloc ;
- (id)description ;
- (id)_init ;
- (void)_addTouch:(id)a0 forDelayedDelivery:(BOOL)a1 ;
- (id)touchesForGestureRecognizer:(id)a0 ;
- (void)_clearViewForTouch:(id)a0 ;
- (void)_removeTouch:(id)a0 ;
- (id)allTouches ;
- (id)_windows ;
- (id)_cloneEvent ;
- (void)_setHIDEvent:(__IOHIDEvent *)a0 ;
- (void)_moveCoalescedTouchesFromHidEvent:(__IOHIDEvent *)a0 toHidEvent:(__IOHIDEvent *)a1 ;
- (id)_lastPendingCoalescedTouchForIndex:(long)a0 hidEvent:(__IOHIDEvent *)a1 ;
- (void)_addCoalescedTouch:(id)a0 forIndex:(long)a1 hidEvent:(__IOHIDEvent *)a2 ;
- (id)_windowsAwaitingLatentSystemGestureNotificationDeliveredToEventWindow:(id)a0 ;
- (id)_allTouches ;
- (void)_clearTouches ;
- (BOOL)_anyInterestedGestureRecognizersForTouchInView:(id)a0 ;
- (CGPoint)_digitizerLocation ;
- (id)_gestureRecognizersForWindow:(id)a0 ;
- (void)_touchesForGesture:(id)a0 withPhase:(long)a1 intoSet:(id)a2 ;
- (id)_viewsForWindow:(id)a0 ;
- (id)_touchesForView:(id)a0 withPhase:(long)a1 ;
- (void)_invalidateGestureRecognizerForWindowCache ;
- (id)_firstTouchForView:(id)a0 ;
- (id)touchesForWindow:(id)a0 ;
- (id)touchesForView:(id)a0 ;
- (id)coalescedTouchesForTouch:(id)a0 ;
- (id)predictedTouchesForTouch:(id)a0 ;
- (id)_initWithEvent:(__GSEvent *)a0 touches:(id)a1 ;
- (id)_touchesForGestureRecognizer:(id)a0 ;
- (id)_touchesForKey:(id)a0 ;
- (id)_touchesForWindow:(id)a0 ;
- (void)_addGestureRecognizersForView:(id)a0 toTouch:(id)a1 ;
- (void)_clearObservedTouchesForTouch:(id)a0 ;
- (id)_rawCoalescedTouchesForTouch:(id)a0 ;
- (void)_observeTouch:(id)a0 ;
- (void)_removeTouchesForKey:(id)a0 ;
- (void)_removeTouchesForWindow:(id)a0 ;
- (__CFDictionary *)_coalescedTouchesForHidEvent:(__IOHIDEvent *)a0 ;
- (void)_removeTouch:(id)a0 fromGestureRecognizer:(id)a1 ;
- (id)_touchesForGesture:(id)a0 withPhase:(long)a1 ;
- (void)_moveTouchesFromView:(id)a0 toView:(id)a1 ;
- (CGFloat)_initialTouchTimestamp ;
- (unsigned)_windowServerHitTestContextId ;
- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 ;
- (void)_windowNoLongerAwaitingSystemGestureNotification:(id)a0 ;
- (long)type ;

@end

