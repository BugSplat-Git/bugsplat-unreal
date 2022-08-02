//
//  BugsplatAttachment.h
//  Bugsplat
//
//  Created by Geoff Raeder on 3/26/17.
//  Copyright © 2017 Bugsplat. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BugsplatAttachment : NSObject

/**
 The filename the attachment should get
 */
@property (nonatomic, readonly, strong, nonnull) NSString *filename;

/**
 The attachment data as NSData object
 */
@property (nonatomic, readonly, strong, nonnull) NSData *attachmentData;

/**
 The content type of your data as MIME type
 */
@property (nonatomic, readonly, strong, nonnull) NSString *contentType;

/**
 Create an BugsplatAttachment instance with a given filename and NSData object
 
 @param filename             The filename the attachment should get. If nil will get a automatically generated filename
 @param attachmentData       The attachment data as NSData. The instance will be ignore if this is set to nil!
 @param contentType          The content type of your data as MIME type. If nil will be set to "application/octet-stream"
 
 @return An instsance of BugsplatAttachment
 */
- (nonnull instancetype)initWithFilename:(nonnull NSString *)filename attachmentData:(nonnull NSData *)attachmentData contentType:(nonnull NSString *)contentType;

@end
