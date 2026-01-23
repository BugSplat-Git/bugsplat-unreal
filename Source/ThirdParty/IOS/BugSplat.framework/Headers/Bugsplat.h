//
//  BugSplat.h
//
//  Copyright © BugSplat, LLC. All rights reserved.
//

#import <TargetConditionals.h>
#import <Foundation/Foundation.h>

//! Project version number for BugSplat.
FOUNDATION_EXPORT double BugSplatVersionNumber;

//! Project version string for BugSplat.
FOUNDATION_EXPORT const unsigned char BugSplatVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <BugSplat/PublicHeader.h>

#if TARGET_OS_OSX
  #import <BugSplatMac/BugSplatDelegate.h>
  #import <BugSplatMac/BugSplatAttachment.h>
#else
  #import <BugSplat/BugSplatDelegate.h>
  #import <BugSplat/BugSplatAttachment.h>
#endif

NS_ASSUME_NONNULL_BEGIN

/// App's Info.plist String entry which is a customer specific BugSplat database name where crash reports will be uploaded.
/// e.g: "fred" (which will reference the https://fred.bugsplat.com/ database)
#define kBugSplatDatabase @"BugSplatDatabase"


@protocol BugSplatDelegate;

@interface BugSplat : NSObject

/*!
 *  BugSplat singleton initializer/accessor
 *
 *  @return shared instance of BugSplat
 */
+ (instancetype)shared;

/*!
 *  Configures and starts crash reporting service
 */
- (void)start;

/**
 * Set the delegate
 *
 * Defines the class that implements the optional protocol `BugSplatDelegate`.
 *
 * @see BugSplatDelegate
 */
@property (weak, nonatomic, nullable) id<BugSplatDelegate> delegate;

/**
 * The database name BugSplat will use to construct the BugSplatDatabase URL where crash reports will be submitted.
 *
 * By default, the BugSplat database name is pulled from the App's Info.plist (BugSplatDatabase key).
 *
 * Set this property to override the Info.plist value or to set the database programmatically.
 * The value can be changed at any time and will be captured at crash time.
 *
 * @note If neither Info.plist nor this property is set before calling `start`, an assertion will fail.
 */
@property (nonatomic, copy, nullable) NSString *bugSplatDatabase;

/**
 * The application name that will be used when a crash report is submitted.
 *
 * By default, this value is pulled from the App's Info.plist (CFBundleDisplayName or CFBundleName).
 *
 * Set this property to override the default application name. The value can be changed at any time
 * and will be captured at crash time.
 */
@property (nonatomic, copy, nullable) NSString *applicationName;

/**
 * The application version that will be used when a crash report is submitted.
 *
 * By default, this value is pulled from the App's Info.plist (CFBundleShortVersionString).
 *
 * Set this property to override the default application version. The value can be changed at any time
 * and will be captured at crash time.
 */
@property (nonatomic, copy, nullable) NSString *applicationVersion;

/**
 * The user name that will be used when a crash report is submitted.
 *
 * The value can be set programmatically at any time and will be stored in NSUserDefaults.
 * To delete the value from NSUserDefaults, set the value to `nil`.
 *
 * This property is optional.
 *
 * @warning When returning a non nil value, crash reports are not anonymous any more
 * and the crash alerts will not show the word "anonymous"!
 *
 * @warning If setting this property programmatically, it needs to be set before calling `start`
 * if the userName should be included in a possible crash from the last app session.
 *
 * @see userEmail
 */
@property (nonatomic, copy, nullable) NSString *userName;

/**
 * The user email address that will be used when a crash report is submitted.
 *
 * The value can be set programmatically at any time and will be stored in NSUserDefaults.
 * To delete the value from NSUserDefaults, set the value to `nil`.
 *
 * This property is optional.
 *
 * @warning When returning a non nil value, crash reports are not anonymous any more
 * and the crash alerts will not show the word "anonymous"!
 *
 * @warning If setting this property programmatically, it needs to be set before calling `start`
 * if the userEmail should be included in a possible crash from the last app session.
 *
 * @see userName
 */
@property (nonatomic, copy, nullable) NSString *userEmail;

/**
 * An application-defined key that will be included with crash reports.
 *
 * Use this for any identifier meaningful to your application, such as:
 * - License keys
 * - Build identifiers
 * - User segments
 * - Environment identifiers (dev/staging/prod)
 *
 * In the BugSplat dashboard, you can configure custom localized support responses
 * for crash groups based on the appKey value.
 *
 * The value is captured at crash time and included with the crash report.
 */
@property (nonatomic, copy, nullable) NSString *appKey;

/**
 * Additional notes to include with crash reports.
 *
 * Use this for any additional context about the crash, such as:
 * - Build configuration details
 * - Feature flags enabled
 * - Recent user actions
 * - Debug information
 *
 * The value is captured at crash time and included with the crash report.
 */
@property (nonatomic, copy, nullable) NSString *notes;

/*!
 *  Submit crash reports without asking the user
 *
 *  _YES_: The crash report will be submitted without asking the user
 *  _NO_: The user will be asked if the crash report can be submitted
 *
 *  On iOS, when set to NO, the user is presented with an alert with options:
 *  - "Send" - sends this crash report
 *  - "Don't Send" - discards all pending crash reports
 *  - "Always Send" - sends this crash report and enables auto-submit for future crashes
 *
 *  Default: iOS: _YES_, macOS: _NO_
 */
@property (nonatomic, assign) BOOL autoSubmitCrashReport;

/**
 * Add an attribute and value to a dictionary of attributes that will potentially be included in a crash report.
 * If the attribute is an invalid XML entity name, or the attribute+value pair cannot be set,
 * the method will return NO, otherwise it will return YES.
 *
 * Attributes and values represent app supplied keys and values to associate with a crash report, should the app crash during this session.
 * Attributes and values will be bundled up in a BugSplatAttachment as NSData, with a filename of CrashContext.xml, MIME type of "application/xml" and encoding of "UTF-8".
 *
 * IMPORTANT: For iOS, only one BugSplatAttachment is currently supported.
 * If BugSplatDelegate's method `- (BugSplatAttachment *)attachmentForBugSplat:(BugSplat *)bugSplat` returns a non-nil BugSplatAttachment,
 * BugSplat will send that BugSplatAttachment, not the BugSplatAttachment that would otherwise be created due to adding attributes and values using this method.
 *
 * NOTES:
 *
 * This method may be called multiple times, once per attribute+value pair.
 * This method may be called at any time during the app session prior to a crash.
 * Attributes are persisted to NSUserDefaults within a NSDictionary<NSString *, NSString *>, so attribute names must be unique.
 * If the attribute does not exist, it will be added to attributes dictionary.
 * If attribute already exists, the value will be replaced in the dictionary.
 * If attribute already exists, and the value is nil, the attribute will be removed from the dictionary.
 *
 * When this method is called, the following preprocessing occurs:
 * 1. attribute will be checked for XML entity name rules. If validation fails, method returns NO.
 *
 * 2. values will then be processed by an XML escaping routine which looks for escapable characters ",',&,<, and >
 * See: https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
 * Any XML comment blocks or CDATA blocks found will disable XML escaping within the block.
 *
 * 3. After processing both attribute and value for XML escape characters, the attribute+value pair will be
 * persisted to NSUserDefaults within a NSDictionary<NSString *, NSString *>.
 *
 * 4. If the attribute or value cannot be set, the method will return NO, otherwise it will return YES.
 *
 * If a crash occurs, attributes and values will be bundled up in a BugSplatAttachment as NSData, with a filename of CrashContext.xml, MIME type of "application/xml"
 * and encoding of "UTF-8". The attachment will be included with the crash data (except as noted above regarding iOS BugSplatAttachment limitation).
 *
 * Attributes and their values are only valid for the lifetime of the app session and only used in a crash report if the crash occurs during that app session.
 * Any attributes set in the prior app session will be bundled up in a BugSplatAttachment as NSData, with a filename of CrashContext.xml,
 * MIME type of "application/xml" and encoding of "UTF-8". The attachment will be added to the crash report when it is processed during the next launch of the app.
 * If the app terminates normally, any attributes persisted during the prior `normal` app session will be erased during the next app launch.
 *
 */
- (BOOL)setValue:(nullable NSString *)value forAttribute:(NSString *)attribute NS_SWIFT_NAME(set(_:for:));

// macOS specific API
#if TARGET_OS_OSX
/*!
 *  Provide custom banner image for crash reporter.
 *  Can set directly in code or provide an image named bugsplat-logo in main bundle. Can be in asset catalog.
 */
@property (nonatomic, strong, nullable) NSImage *bannerImage;

/**
 *  Defines if the crash report UI should ask for name and email
 *
 *  Default: _YES_
 */
@property (nonatomic, assign) BOOL askUserDetails;

/**
 * If the user enters their name or email on a Bug Crash Alert Form, persist their data to NSUserDefaults.
 * After this occurs, userName and userEmail properties will contain the values the user entered.
 * When the Bug Crash Alert Form is presented again, it will be pre-populated with user name and email.
 * To erase their user name or email, set the property value to nil programmatically.
 *
 * This property defaults to NO.
 * This property is optional.
 *
 * @warning If setting this property to YES, it needs to be set before calling `start`.
 *
 * @see userName
 * @see userEmail
 */
@property (nonatomic, assign) BOOL persistUserDetails;

/**
 *  Defines if crash reports should be considered "expired" after a certain amount of time (in seconds).
 *  If expired crash dialogue is not displayed but reports are still uploaded.
 *
 *  Default: -1 // No expiration
 */
@property (nonatomic, assign) NSTimeInterval expirationTimeInterval;

/**
 * Option to present crash reporter dialogue modally
 *
 * *Default*:  NO
 */
@property (nonatomic, assign) BOOL presentModally;

#endif

@end

NS_ASSUME_NONNULL_END
