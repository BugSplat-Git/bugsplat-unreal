//
//  BugsplatStartupManager.h
//  Bugsplat
//
//  Created by Geoff Raeder on 6/13/21.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol BugsplatStartupManagerDelegate;

@interface BugsplatStartupManager : NSObject

/*!
 *  BugsplatStartupManager singleton initializer/accessor
 *
 *  @return shared instance of BugsplatStartupManager
 */
+ (instancetype)sharedManager;

/*!
 *  Configures and starts crash reporting service
 */
- (void)start;

/**
 * Set the delegate
 *
 * Defines the class that implements the optional protocol `BugsplatStartupManagerDelegate`.
 *
 * @see BugsplatStartupManagerDelegate
 */
@property (weak, nonatomic, nullable) id<BugsplatStartupManagerDelegate> delegate;

/** Set the userid that should used in the SDK components

 Right now this is used by the Crash Manager to attach to a crash report.

 The value can be set at any time and will be stored in the keychain on the current
 device only! To delete the value from the keychain set the value to `nil`.

 This property is optional.

 @warning When returning a non nil value, crash reports are not anonymous any more
 and the crash alerts will not show the word "anonymous"!

 @warning This property needs to be set before calling `startManager` to be considered
 for being added to crash reports as meta data.

 @see userName
 @see userEmail
 @see `[BITHockeyManagerDelegate userIDForHockeyManager:componentManager:]`
 */
@property (nonatomic, copy, nullable) NSString *userID;

/** Set the user name that should used in the SDK components

 Right now this is used by the Crash Manager to attach to a crash report.

 The value can be set at any time and will be stored in the keychain on the current
 device only! To delete the value from the keychain set the value to `nil`.

 This property is optional.

 @warning When returning a non nil value, crash reports are not anonymous any more
 and the crash alerts will not show the word "anonymous"!

 @warning This property needs to be set before calling `startManager` to be considered
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

 @warning This property needs to be set before calling `startManager` to be considered
 for being added to crash reports as meta data.

 @see userID
 @see userName
 @see [BITHockeyManagerDelegate userEmailForHockeyManager:componentManager:]
 */
@property (nonatomic, copy, nullable) NSString *userEmail;

@end

NS_ASSUME_NONNULL_END
