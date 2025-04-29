//
//  BugSplatDelegate.h
//
//  Copyright © BugSplat, LLC. All rights reserved.
//

#if TARGET_OS_OSX
  #import <BugSplatMac/BugSplatMac.h>
#else
  #import <BugSplat/BugSplat.h>
#endif

NS_ASSUME_NONNULL_BEGIN

@class BugSplat;
@class BugSplatAttachment;

@protocol BugSplatDelegate <NSObject>

@optional

// MARK: - BugSplatDelegate (MacOS, iOS)

/** Return any log string based data the crash report being processed should contain
 *
 * @param bugSplat The `BugSplat` instance invoking this delegate
 */
- (NSString *)applicationLogForBugSplat:(BugSplat *)bugSplat;

/** Invoked right before sending crash reports will start

 @param bugSplat The `BugSplat` instance invoking this delegate
 */
- (void)bugSplatWillSendCrashReport:(BugSplat *)bugSplat;

/** Invoked after sending crash reports failed

 @param bugSplat The `BugSplat` instance invoking this delegate
 @param error The error returned from the NSURLSession call
 */
- (void)bugSplat:(BugSplat *)bugSplat didFailWithError:(NSError *)error;

/** Invoked after sending crash reports succeeded

 @param bugSplat The `BugSplat` instance invoking this delegate
 */
- (void)bugSplatDidFinishSendingCrashReport:(BugSplat *)bugSplat;

/** Invoked before the user is asked to send a crash report, so you can do additional actions.
 E.g. to make sure not to ask the user for an app rating :)

 @param bugSplat The `BugSplat` instance invoking this delegate
 */
-(void)bugSplatWillShowSubmitCrashReportAlert:(BugSplat *)bugSplat;

/** Invoked after the user did choose _NOT_ to send a crash in the alert

 @param bugSplat The `BugSplat` instance invoking this delegate
 */
-(void)bugSplatWillCancelSendingCrashReport:(BugSplat *)bugSplat;

/** Return a BugSplatAttachment object providing an NSData object the crash report being processed should contain
 NOTE: For iOS, if this method returns a non-nil BugSplatAttachment, any attributes added via setAttribute:value: to BugSplat will NOT be included in the Crash Report.

 Example implementation:

 NSData *data = [NSData dataWithContentsOfURL:@"mydatafile"];

 BugSplatAttachment *attachment = [[BugSplatAttachment alloc] initWithFilename:@"myfile.data"
                                                                attachmentData:data
                                                                   contentType:@"application/octet-stream"];
 @param bugSplat The `BugSplat` instance invoking this delegate
*/
- (BugSplatAttachment *)attachmentForBugSplat:(BugSplat *)bugSplat API_AVAILABLE(ios(13.0));

// MARK: - BugSplatDelegate (MacOS)
#if TARGET_OS_OSX

/** Return any string based data the crash report being processed should contain
 *
 * @param bugSplat The `BugSplat` instance invoking this delegate
 * @param signal The system crash signal
 * @param exceptionName The exception name.  Only provided if crash is the result of an uncaught exception
 * @param exceptionReason The exception reason.  Only provided if crash is the result of an uncaught exception
 */
- (NSString *)applicationKeyForBugSplat:(BugSplat *)bugSplat signal:(NSString *)signal exceptionName:(NSString *)exceptionName exceptionReason:(NSString *)exceptionReason API_AVAILABLE(macosx(10.13));

/** Return a collection of BugsplatAttachment objects providing an NSData object the crash report being processed should contain

 Example implementation:

 NSData *data = [NSData dataWithContentsOfURL:@"mydatafile"];

 BugsplatAttachment *attachment = [[BugsplatAttachment alloc] initWithFilename:@"myfile.data"
                                                                attachmentData:data
                                                                   contentType:@"application/octet-stream"];

 @param bugSplat The `BugSplat` instance invoking this delegate
*/
- (NSArray<BugSplatAttachment *> *)attachmentsForBugSplat:(BugSplat *)bugSplat API_AVAILABLE(macosx(10.13));

// MARK: - BugSplatDelegate (iOS)
#else

/** Invoked after the user did choose to send crashes always in the alert

 @param bugSplat The `BugSplat` instance invoking this delegate
 */
-(void)bugSplatWillSendCrashReportsAlways:(BugSplat *)bugSplat API_AVAILABLE(ios(13.0));

#endif

@end

NS_ASSUME_NONNULL_END
