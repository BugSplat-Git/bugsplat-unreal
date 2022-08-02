//
//  BugsplatStartupManagerDelegate.h
//  Bugsplat
//
//  Created by Geoff Raeder on 6/13/21.
//


NS_ASSUME_NONNULL_BEGIN

@class BugsplatAttachment;

@protocol BugsplatStartupManagerDelegate <NSObject>

@optional

/** Return any log string based data the crash report being processed should contain
 *
 * @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 */
- (NSString *)applicationLogForBugsplat:(BugsplatStartupManager *)bugsplatStartupManager;

/** Return a collection of BugsplatAttachment objects providing an NSData object the crash report
 being processed should contain

 Example implementation:

 NSData *data = [NSData dataWithContentsOfURL:@"mydatafile"];

 BugsplatAttachment *attachment = [[BugsplatAttachment alloc] initWithFilename:@"myfile.data"
                                                                attachmentData:data
                                                                   contentType:@"application/octet-stream"];

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
*/
- (BugsplatAttachment *)attachmentForBugsplat:(BugsplatStartupManager *)bugsplatStartupManager;

/** Invoked before the user is asked to send a crash report, so you can do additional actions.
 E.g. to make sure not to ask the user for an app rating :)

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 */
-(void)bugsplatWillShowSubmitCrashReportAlert:(BugsplatStartupManager *)bugsplatStartupManager;

/** Invoked after the user did choose _NOT_ to send a crash in the alert

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 */
-(void)bugsplatWillCancelSendingCrashReport:(BugsplatStartupManager *)bugsplatStartupManager;

/** Invoked after the user did choose to send crashes always in the alert

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 */
-(void)bugsplatWillSendCrashReportsAlways:(BugsplatStartupManager *)bugsplatStartupManager;

/** Invoked right before sending crash reports will start

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 */
- (void)bugsplatWillSendCrashReport:(BugsplatStartupManager *)bugsplatStartupManager;

/** Invoked after sending crash reports failed

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 @param error The error returned from the NSURLSession call
 */
- (void)bugsplat:(BugsplatStartupManager *)bugsplatStartupManager didFailWithError:(NSError *)error;

/** Invoked after sending crash reports succeeded

 @param bugsplatStartupManager The `BugsplatStartupManager` instance invoking this delegate
 */
- (void)bugsplatDidFinishSendingCrashReport:(BugsplatStartupManager *)bugsplatStartupManager;

@end

NS_ASSUME_NONNULL_END
