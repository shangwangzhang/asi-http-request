//
//  ASIS3Request.h
//  Part of ASIHTTPRequest -> http://allseeing-i.com/ASIHTTPRequest
//
//  Created by Ben Copsey on 30/06/2009.
//  Copyright 2009 All-Seeing Interactive. All rights reserved.
//
// A class for accessing data stored on Amazon's Simple Storage Service (http://aws.amazon.com/s3/) using the REST API
// While you can use this class directly, the included subclasses make typical operations easier

#import <Foundation/Foundation.h>
#import "ASIHTTPRequest.h"

// See http://docs.amazonwebservices.com/AmazonS3/2006-03-01/index.html?RESTAccessPolicy.html for what these mean
extern NSString *const ASIS3AccessPolicyPrivate; // This is the default in S3 when no access policy header is provided
extern NSString *const ASIS3AccessPolicyPublicRead;
extern NSString *const ASIS3AccessPolicyPublicReadWrote;
extern NSString *const ASIS3AccessPolicyAuthenticatedRead;

typedef enum _ASIS3ErrorType {
    ASIS3ResponseParsingFailedType = 1,
    ASIS3ResponseErrorType = 2
	
} ASIS3ErrorType;

// Prevent warning about missing NSXMLParserDelegate on Leopard and iPhone
#if !TARGET_OS_IPHONE && MAC_OS_X_VERSION_10_5 < MAC_OS_X_VERSION_MAX_ALLOWED
@interface ASIS3Request : ASIHTTPRequest <NSCopying, NSXMLParserDelegate> {
#else
@interface ASIS3Request : ASIHTTPRequest <NSCopying> {
#endif
	// Your S3 access key. Set it on the request, or set it globally using [ASIS3Request setSharedAccessKey:]
	NSString *accessKey;
	
	// Your S3 secret access key. Set it on the request, or set it globally using [ASIS3Request setSharedSecretAccessKey:]
	NSString *secretAccessKey;
	
	// The string that will be used in the HTTP date header. Generally you'll want to ignore this and let the class add the current date for you, but the accessor is used by the tests
	NSString *dateString;

	// The access policy to use when PUTting a file (see the string constants at the top ASIS3Request.h for details on what the possible options are)
	NSString *accessPolicy;

	// Internally used while parsing errors
	NSString *currentErrorString;
	
}

#pragma mark Constructors


// Uses the supplied date to create a Date header string
- (void)setDate:(NSDate *)date;

- (NSMutableDictionary *)S3Headers;
- (NSString *)stringToSignForHeaders:(NSString *)canonicalizedAmzHeaders resource:(NSString *)canonicalizedResource;

	
# pragma mark encoding S3 key
	
+ (NSString *)stringByURLEncodingForS3Path:(NSString *)key;
	
#pragma mark Shared access keys

// Get and set the global access key, this will be used for all requests the access key hasn't been set for
+ (NSString *)sharedAccessKey;
+ (void)setSharedAccessKey:(NSString *)newAccessKey;
+ (NSString *)sharedSecretAccessKey;
+ (void)setSharedSecretAccessKey:(NSString *)newAccessKey;
- (void)parseResponseXML;
	
@property (retain) NSString *dateString;
@property (retain) NSString *accessKey;
@property (retain) NSString *secretAccessKey;
@property (retain) NSString *accessPolicy;
@end
