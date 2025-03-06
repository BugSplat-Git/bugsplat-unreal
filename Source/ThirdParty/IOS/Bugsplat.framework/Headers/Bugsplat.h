//
//  BugSplat.h
//
//  Copyright © 2024 BugSplat, LLC. All rights reserved.
//

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

/** Set the userID that should used in the SDK components

 Right now this is used by the Crash Manager to attach to a crash report.

 The value can be set at any time and will be stored in the keychain on the current
 device only! To delete the value from the keychain set the value to `nil`.

 This property is optional.

 @warning When returning a non nil value, crash reports are not anonymous any more
 and the crash alerts will not show the word "anonymous"!

 @warning This property needs to be set before calling `start` to be considered
 for being added to crash reports as meta data.

 @see userName
 @see userEmail
 @see `[BITHockeyManagerDelegate userIDForHockeyManager:componentManager:]`
 */
@property (nonatomic, copy, nullable) NSString *userID;

/**
 * The database name BugSplat will use to construct the BugSplatDatabase URL where crash reports will be submitted.
 *
 * NOTES:
 * By default, the BugSplat database name is pulled from the App's Info.plist.
 *
 * When a third party library or plugin developer is leveraging BugSplat, but the App developer incorporating
 * the plugin is not using BugSplat, programmatically setting this property would be appropriate.
 *
 * Only one BugSplat database can be specified within an App including any third party libraries and plugins.
 * This means if the Info.plist contains a BugSplat database, attempting to change this property will have no effect.
 *
 * Additionally, if the Info.plist does not contain a BugSplat database key and value, the first call to
 * set this property will set the BugSplat database name. Any subsequent calls to set this property will have no effect.
 *
 * Finally, with the above considerations, if programmatic change of this property is desired, it must be set before calling
 * BugSplat `start` method or it will have no effect.
 */
@property (nonatomic, copy, nullable) NSString *bugSplatDatabase;

/** Set the user name that should used in the SDK components

 Right now this is used by the Crash Manager to attach to a crash report.

 The value can be set at any time and will be stored in the keychain on the current
 device only! To delete the value from the keychain set the value to `nil`.

 This property is optional.

 @warning When returning a non nil value, crash reports are not anonymous any more
 and the crash alerts will not show the word "anonymous"!

 @warning This property needs to be set before calling `start` to be considered
 for being added to crash reports as meta data.

 @see userID
 @see userEmail
 @see `[BITHockeyManagerDelegate userNameForHockeyManager:componentManager:]`
 */
@property (nonatomic, copy, nullable) NSString *userName;

/** Set the users email address that should used in the SDK components

 Right now this is used by the Crash Manager to attach to a crash report.

 The value can be set at any time and will be stored in the keychain on the current
 device only! To delete the value from the keychain set the value to `nil`.

 This property is optional.

 @warning When returning a non nil value, crash reports are not anonymous any more
 and the crash alerts will not show the word "anonymous"!

 @warning This property needs to be set before calling `start` to be considered
 for being added to crash reports as meta data.

 @see userID
 @see userName
 @see [BITHockeyManagerDelegate userEmailForHockeyManager:componentManager:]
 */
@property (nonatomic, copy, nullable) NSString *userEmail;

/*!
 *  Submit crash reports without asking the user
 *
 *  _YES_: The crash report will be submitted without asking the user
 *  _NO_: The user will be asked if the crash report can be submitted (default)
 *
 *  Default: iOS: _YES_, macOS: _NO_
 */
@property (nonatomic, assign) BOOL autoSubmitCrashReport;

/**
 * Add an attribute and value to the crash report.
 * Attributes and values represent app supplied keys and values to associate with a crash report.
 * Attributes and values will be bundled up in a BugSplatAttachment as NSData, with a filename of CrashContext.xml, MIME type of "application/xml" and encoding of "UTF-8".
 *
 * IMPORTANT: For iOS, only one BugSplatAttachment is currently supported.
 * If BugSplatDelegate's method `- (BugSplatAttachment *)attachmentForBugSplat:(BugSplat *)bugSplat` returns a non-nil BugSplatAttachment,
 * BugSplat will send that BugSplatAttachment, not the BugSplatAttachment that would otherwise be created due to adding attributes and values using this method.
 *
 * NOTES:
 *
 * This method may be called multiple times, once per attribute+value pair.
 * Attributes are backed by an NSDictionary so attribute names must be unique.
 * If the attribute does not exist, it will be added to attributes dictionary.
 * If attribute already exists, the value will be replaced in the dictionary.
 * If attribute already exists, and the value is nil, the attribute will be removed from the dictionary.
 *
 * When this method is called, the following preprocessing occurs:
 * 1. attribute will first have white space and newlines removed from both the beginning and end of the String.
 *
 * 2. attribute will then be processed by an XML escaping routine which looks for escapable characters ",',&,<, and >
 * See: https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
 * Any XML comment blocks or CDATA blocks found will disable XML escaping within the block.
 *
 * 3. values will then be processed by an XML escaping routine which looks for escapable characters ",',&,<, and >
 * Any XML comment blocks or CDATA blocks found will disable XML escaping within the block.
 *
 * 4. After processing both attribute and value for XML escape characters, the attribute+value pair will be stored in an NSDictionary.
 *
 * If a crash occurs, attributes and values will be bundled up in a BugSplatAttachment as NSData, with a filename of CrashContext.xml, MIME type of "application/xml"
 * and encoding of "UTF-8". The attachment will be included with the crash data (except as noted above regarding iOS BugSplatAttachment limitation).
 * 
 */
- (void)setValue:(nullable NSString *)value forAttribute:(NSString *)attribute;

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
 *  Defines if user's name and email entered in the crash report UI should be saved to the keychain.
 *
 *  Default: _NO_
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
