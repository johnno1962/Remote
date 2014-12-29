//
//  RemoteHeaders.h
//  Remote
//

#import <UIKit/UIKit.h>

@interface UITouch ()

- (id)description ;
- (void)dealloc ;
- (CGPoint)locationInView:(UIView *)a0 ;
- (void)setWindow:(id)a0 ;
- (void)setIsTap:(BOOL)a0 ;
- (UITouchPhase)phase ;
- (BOOL)isTap ;
- (void)setTapCount:(unsigned long)a0 ;
- (BOOL)_isEaten ;
- (void)setPhase:(long)a0 ;
- (void)_setEaten:(BOOL)a0 ;
- (NSArray *)gestureRecognizers ;
- (void)_setForwardablePhase:(long)a0 ;
- (id)_responder ;
- (void)_setResponder:(id)a0 ;
- (BOOL)sentTouchesEnded ;
- (id)_windowServerHitTestWindow ;
- (CGSize)_displacement ;
- (unsigned char)_pathIndex ;
- (void)_setDisplacement:(CGSize)a0 ;
- (void)_setWindowServerHitTestWindow:(id)a0 ;
- (void)_setEdgeType:(long)a0 ;
- (void)setView:(id)a0 ;
- (id)warpedIntoView ;
- (void)setWarpedIntoView:(id)a0 ;
- (void)_setLocationInWindow:(CGPoint)a0 resetPrevious:(BOOL)a1 ;
- (void)_setPressure:(double)a0 resetPrevious:(BOOL)a1 ;
- (void)_setIsFirstTouchForView:(BOOL)a0 ;
- (void)_updateMovementMagnitudeForLocation:(CGPoint)a0 ;
- (double)_pressure ;
- (void)_setPathIndex:(unsigned char)a0 ;
- (void)_setPathIdentity:(unsigned char)a0 ;
- (void)setMajorRadius:(double)a0 ;
- (void)setMajorRadiusTolerance:(double)a0 ;
- (void)_clearGestureRecognizers ;
- (void)_addGestureRecognizer:(id)a0 ;
- (void)setSentTouchesEnded:(BOOL)a0 ;
- (long)_edgeType ;
- (id)_gestureRecognizers ;
- (BOOL)isDelayed ;
- (CGPoint)previousLocationInView:(UIView *)a0 ;
- (NSUInteger)tapCount ;
- (id)_forwardingRecord ;
- (SEL)_responderSelectorForPhase:(long)a0 ;
- (BOOL)_wantsForwardingFromResponder:(id)a0 toNextResponder:(id)a1 withEvent:(id)a2 ;
- (CGPoint)_locationInSceneReferenceSpace ;
- (void)_removeGestureRecognizer:(id)a0 ;
- (BOOL)_isFirstTouchForView ;
- (long)_forwardablePhase ;
- (BOOL)_isAbandoningForwardingRecord ;
- (id)_mutableForwardingRecord ;
- (void)_abandonForwardingRecord ;
- (CGFloat)majorRadius ;
- (void)setIsDelayed:(BOOL)a0 ;
- (void)_loadStateFromTouch:(id)a0 ;
- (float)_pathMajorRadius ;
- (unsigned char)_pathIdentity ;
- (CGFloat)majorRadiusTolerance ;
- (id)_phaseChangeDelegate ;
- (void)_setPhaseChangeDelegate:(id)a0 ;
- (id)_phaseDescription ;
- (CGPoint)_locationInWindow:(id)a0 ;
- (CGPoint)_previousLocationInWindow:(id)a0 ;
- (void)_pushPhase:(long)a0 ;
- (void)_popPhase ;
- (CGPoint)_previousLocationInSceneReferenceSpace ;
- (long)_compareIndex:(id)a0 ;
- (BOOL)_isStationaryRelativeToTouches:(id)a0 ;
- (double)_distanceFrom:(id)a0 inView:(id)a1 ;
- (UIWindow *)window ;
- (long)info ;
- (void)setTimestamp:(double)a0 ;
- (UIView *)view ;
- (double)timestamp ;

@end

@class __GSEvent;
@class __CFDictionary;

@interface UITouchesEvent : NSObject

- (id)_init ;
- (id)description ;
- (void)dealloc ;
- (void)_addTouch:(id)a0 forDelayedDelivery:(BOOL)a1 ;
- (id)touchesForGestureRecognizer:(id)a0 ;
- (void)_clearViewForTouch:(id)a0 ;
- (void)_removeTouch:(id)a0 ;
- (id)allTouches ;
- (id)_windows ;
- (id)_windowsAwaitingLatentSystemGestureNotificationDeliveredToEventWindow:(id)a0 ;
- (id)_allTouches ;
- (void)_clearTouches ;
- (void)_dismissSharedCalloutBarIfNeeded ;
- (BOOL)_addGestureRecognizersForView:(id)a0 toTouch:(id)a1 currentTouchMap:(__CFDictionary *)a2 newTouchMap:(__CFDictionary *)a3 ;
- (id)_touchesForKey:(id)a0 ;
- (id)_gestureRecognizersForWindow:(id)a0 ;
- (void)_touchesForGesture:(id)a0 withPhase:(long)a1 intoSet:(id)a2 ;
- (id)_viewsForWindow:(id)a0 ;
- (id)_touchesForView:(id)a0 withPhase:(long)a1 ;
- (void)_invalidateGestureRecognizerForWindowCache ;
- (id)_firstTouchForView:(id)a0 ;
- (id)_cloneEvent ;
- (id)touchesForWindow:(id)a0 ;
- (id)touchesForView:(id)a0 ;
- (id)_initWithEvent:(__GSEvent *)a0 touches:(id)a1 ;
- (id)_touchesForGestureRecognizer:(id)a0 ;
- (id)_touchesForWindow:(id)a0 ;
- (void)_removeTouchesForKey:(id)a0 ;
- (void)_removeTouchesForWindow:(id)a0 ;
- (void)_removeTouch:(id)a0 fromGestureRecognizer:(id)a1 ;
- (id)_touchesForGesture:(id)a0 withPhase:(long)a1 ;
- (void)_moveTouchesFromView:(id)a0 toView:(id)a1 ;
- (double)_initialTouchTimestamp ;
- (unsigned)_windowServerHitTestContextId ;
- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 ;
- (void)_windowNoLongerAwaitingSystemGestureNotification:(id)a0 ;
- (long)type ;

@end

