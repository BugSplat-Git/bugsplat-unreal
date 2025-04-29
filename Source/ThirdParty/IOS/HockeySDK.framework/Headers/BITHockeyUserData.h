//
//  BITHockeyUserData.h
//  HockeySDK
//

#import <Foundation/Foundation.h>


@interface BITHockeyUserData : NSObject

/**
 *  User provided userName
 */
@property (nonatomic, copy, nullable) NSString *userName;

/**
 *  User provide userEmail
 */
@property (nonatomic, copy, nullable) NSString *userEmail;

/**
 *  User provided description text
 */
@property (nonatomic, copy, nullable) NSString *userProvidedText;

@end
