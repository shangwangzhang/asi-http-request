//
//  ASIHTTPRequest.h
//
//  Created by Ben Copsey on 04/10/2007.
//  Copyright 2007-2010 All-Seeing Interactive. All rights reserved.
//
//  A guide to the main features is available at:
//  http://allseeing-i.com/ASIHTTPRequest
//
//  Portions are based on the ImageClient example from Apple:
//  See: http://developer.apple.com/samplecode/ImageClient/listing37.html

#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
	#import <CFNetwork/CFNetwork.h>
#endif

#import <stdio.h>
#import "ASIHTTPRequestConfig.h"
#import "ASIHTTPRequestDelegate.h"
#import "ASIProgressDelegate.h"
#import "ASICacheDelegate.h"

@class ASIDataDecompressor;

extern NSString *ASIHTTPRequestVersion;

// Make targeting different platforms more reliable
// See: http://www.blumtnwerx.com/blog/2009/06/cross-sdk-code-hygiene-in-xcode/
#ifndef __IPHONE_3_2
	#define __IPHONE_3_2 30200
#endif
#ifndef __IPHONE_4_0
	#define __IPHONE_4_0 40000
#endif
#ifndef __MAC_10_5
	#define __MAC_10_5 1050
#endif
#ifndef __MAC_10_6
	#define __MAC_10_6 1060
#endif

typedef enum _ASIAuthenticationState {
	ASINoAuthenticationNeededYet = 0,
	ASIHTTPAuthenticationNeeded = 1,
	ASIProxyAuthenticationNeeded = 2
} ASIAuthenticationState;

typedef enum _ASINetworkErrorType {
    ASIConnectionFailureErrorType = 1,
    ASIRequestTimedOutErrorType = 2,
    ASIAuthenticationErrorType = 3,
    ASIRequestCancelledErrorType = 4,
    ASIUnableToCreateRequestErrorType = 5,
    ASIInternalErrorWhileBuildingRequestType  = 6,
    ASIInternalErrorWhileApplyingCredentialsType  = 7,
	ASIFileManagementError = 8,
	ASITooMuchRedirectionErrorType = 9,
	ASIUnhandledExceptionError = 10,
	ASICompressionError = 11
	
} ASINetworkErrorType;

// The error domain that all errors generated by ASIHTTPRequest use
extern NSString* const NetworkRequestErrorDomain;

// You can use this number to throttle upload and download bandwidth in iPhone OS apps send or receive a large amount of data
// This may help apps that might otherwise be rejected for inclusion into the app store for using excessive bandwidth
// This number is not official, as far as I know there is no officially documented bandwidth limit
extern unsigned long const ASIWWANBandwidthThrottleAmount;

@interface ASIHTTPRequest : NSOperation <NSCopying> {
	
	// The url for this operation, should include GET params in the query string where appropriate
	NSURL *url; 
	
	// Will always contain the original url used for making the request (the value of url can change when a request is redirected)
	NSURL *originalURL;
	
	// The delegate, you need to manage setting and talking to your delegate in your subclasses
	id <ASIHTTPRequestDelegate> delegate;
	
	// Another delegate that is also notified of request status changes and progress updates
	// Generally, you won't use this directly, but ASINetworkQueue sets itself as the queue so it can proxy updates to its own delegates
	// NOTE: WILL BE RETAINED BY THE REQUEST
	id <ASIHTTPRequestDelegate, ASIProgressDelegate> queue;
	
	// HTTP method to use (GET / POST / PUT / DELETE / HEAD). Defaults to GET
	NSString *requestMethod;
	
	// Request body - only used when the whole body is stored in memory (shouldStreamPostDataFromDisk is false)
	NSMutableData *postBody;
	
	// gzipped request body used when shouldCompressRequestBody is YES
	NSData *compressedPostBody;
	
	// When true, post body will be streamed from a file on disk, rather than loaded into memory at once (useful for large uploads)
	// Automatically set to true in ASIFormDataRequests when using setFile:forKey:
	BOOL shouldStreamPostDataFromDisk;
	
	// Path to file used to store post body (when shouldStreamPostDataFromDisk is true)
	// You can set this yourself - useful if you want to PUT a file from local disk 
	NSString *postBodyFilePath;
	
	// Path to a temporary file used to store a deflated post body (when shouldCompressPostBody is YES)
	NSString *compressedPostBodyFilePath;
	
	// Set to true when ASIHTTPRequest automatically created a temporary file containing the request body (when true, the file at postBodyFilePath will be deleted at the end of the request)
	BOOL didCreateTemporaryPostDataFile;
	
	// Used when writing to the post body when shouldStreamPostDataFromDisk is true (via appendPostData: or appendPostDataFromFile:)
	NSOutputStream *postBodyWriteStream;
	
	// Used for reading from the post body when sending the request
	NSInputStream *postBodyReadStream;
	
	// Dictionary for custom HTTP request headers
	NSMutableDictionary *requestHeaders;
	
	// Set to YES when the request header dictionary has been populated, used to prevent this happening more than once
	BOOL haveBuiltRequestHeaders;
	
	// Will be populated with HTTP response headers from the server
	NSDictionary *responseHeaders;
	
	// Can be used to manually insert cookie headers to a request, but it's more likely that sessionCookies will do this for you
	NSMutableArray *requestCookies;
	
	// Will be populated with cookies
	NSArray *responseCookies;
	
	// If use useCookiePersistence is true, network requests will present valid cookies from previous requests
	BOOL useCookiePersistence;
	
	// If useKeychainPersistence is true, network requests will attempt to read credentials from the keychain, and will save them in the keychain when they are successfully presented
	BOOL useKeychainPersistence;
	
	// If useSessionPersistence is true, network requests will save credentials and reuse for the duration of the session (until clearSession is called)
	BOOL useSessionPersistence;
	
	// If allowCompressedResponse is true, requests will inform the server they can accept compressed data, and will automatically decompress gzipped responses. Default is true.
	BOOL allowCompressedResponse;
	
	// If shouldCompressRequestBody is true, the request body will be gzipped. Default is false.
	// You will probably need to enable this feature on your webserver to make this work. Tested with apache only.
	BOOL shouldCompressRequestBody;
	
	// When downloadDestinationPath is set, the result of this request will be downloaded to the file at this location
	// If downloadDestinationPath is not set, download data will be stored in memory
	NSString *downloadDestinationPath;
	
	// The location that files will be downloaded to. Once a download is complete, files will be decompressed (if necessary) and moved to downloadDestinationPath
	NSString *temporaryFileDownloadPath;
	
	// If the response is gzipped and shouldWaitToInflateCompressedResponses is NO, a file will be created at this path containing the inflated response as it comes in
	NSString *temporaryUncompressedDataDownloadPath;
	
	// Used for writing data to a file when downloadDestinationPath is set
	NSOutputStream *fileDownloadOutputStream;
	
	NSOutputStream *inflatedFileDownloadOutputStream;
	
	// When the request fails or completes successfully, complete will be true
	BOOL complete;
	
    // external "finished" indicator, subject of KVO notifications; updates after 'complete'
    BOOL finished;
    
    // True if our 'cancel' selector has been called
    BOOL cancelled;
    
	// If an error occurs, error will contain an NSError
	// If error code is = ASIConnectionFailureErrorType (1, Connection failure occurred) - inspect [[error userInfo] objectForKey:NSUnderlyingErrorKey] for more information
	NSError *error;
	
	// Username and password used for authentication
	NSString *username;
	NSString *password;
	
	// Domain used for NTLM authentication
	NSString *domain;
	
	// Username and password used for proxy authentication
	NSString *proxyUsername;
	NSString *proxyPassword;
	
	// Domain used for NTLM proxy authentication
	NSString *proxyDomain;
	
	// Delegate for displaying upload progress (usually an NSProgressIndicator, but you can supply a different object and handle this yourself)
	id <ASIProgressDelegate> uploadProgressDelegate;
	
	// Delegate for displaying download progress (usually an NSProgressIndicator, but you can supply a different object and handle this yourself)
	id <ASIProgressDelegate> downloadProgressDelegate;
	
	// Whether we've seen the headers of the response yet
    BOOL haveExaminedHeaders;
	
	// Data we receive will be stored here. Data may be compressed unless allowCompressedResponse is false - you should use [request responseData] instead in most cases
	NSMutableData *rawResponseData;
	
	// Used for sending and receiving data
    CFHTTPMessageRef request;	
	NSInputStream *readStream;
	
	// Used for authentication
    CFHTTPAuthenticationRef requestAuthentication; 
	NSDictionary *requestCredentials;
	
	// Used during NTLM authentication
	int authenticationRetryCount;
	
	// Authentication scheme (Basic, Digest, NTLM)
	NSString *authenticationScheme;
	
	// Realm for authentication when credentials are required
	NSString *authenticationRealm;
	
	// When YES, ASIHTTPRequest will present a dialog allowing users to enter credentials when no-matching credentials were found for a server that requires authentication
	// The dialog will not be shown if your delegate responds to authenticationNeededForRequest:
	// Default is NO.
	BOOL shouldPresentAuthenticationDialog;
	
	// When YES, ASIHTTPRequest will present a dialog allowing users to enter credentials when no-matching credentials were found for a proxy server that requires authentication
	// The dialog will not be shown if your delegate responds to proxyAuthenticationNeededForRequest:
	// Default is YES (basically, because most people won't want the hassle of adding support for authenticating proxies to their apps)
	BOOL shouldPresentProxyAuthenticationDialog;	
	
	// Used for proxy authentication
    CFHTTPAuthenticationRef proxyAuthentication; 
	NSDictionary *proxyCredentials;
	
	// Used during authentication with an NTLM proxy
	int proxyAuthenticationRetryCount;
	
	// Authentication scheme for the proxy (Basic, Digest, NTLM)
	NSString *proxyAuthenticationScheme;	
	
	// Realm for proxy authentication when credentials are required
	NSString *proxyAuthenticationRealm;
	
	// HTTP status code, eg: 200 = OK, 404 = Not found etc
	int responseStatusCode;
	
	// Description of the HTTP status code
	NSString *responseStatusMessage;
	
	// Size of the response
	unsigned long long contentLength;
	
	// Size of the partially downloaded content
	unsigned long long partialDownloadSize;
	
	// Size of the POST payload
	unsigned long long postLength;	
	
	// The total amount of downloaded data
	unsigned long long totalBytesRead;
	
	// The total amount of uploaded data
	unsigned long long totalBytesSent;
	
	// Last amount of data read (used for incrementing progress)
	unsigned long long lastBytesRead;
	
	// Last amount of data sent (used for incrementing progress)
	unsigned long long lastBytesSent;
	
	// This lock prevents the operation from being cancelled at an inopportune moment
	NSRecursiveLock *cancelledLock;
	
	// Called on the delegate (if implemented) when the request starts. Default is requestStarted:
	SEL didStartSelector;
	
	// Called on the delegate (if implemented) when the request receives response headers. Default is requestDidReceiveResponseHeaders:
	SEL didReceiveResponseHeadersSelector;

	// Called on the delegate (if implemented) when the request completes successfully. Default is requestFinished:
	SEL didFinishSelector;
	
	// Called on the delegate (if implemented) when the request fails. Default is requestFailed:
	SEL didFailSelector;
	
	// Called on the delegate (if implemented) when the request receives data. Default is request:didReceiveData:
	// If you set this and implement the method in your delegate, you must handle the data yourself - ASIHTTPRequest will not populate responseData or write the data to downloadDestinationPath
	SEL didReceiveDataSelector;
	
	// Used for recording when something last happened during the request, we will compare this value with the current date to time out requests when appropriate
	NSDate *lastActivityTime;
	
	// Number of seconds to wait before timing out - default is 10
	NSTimeInterval timeOutSeconds;
	
	// Will be YES when a HEAD request will handle the content-length before this request starts
	BOOL shouldResetUploadProgress;
	BOOL shouldResetDownloadProgress;
	
	// Used by HEAD requests when showAccurateProgress is YES to preset the content-length for this request
	ASIHTTPRequest *mainRequest;
	
	// When NO, this request will only update the progress indicator when it completes
	// When YES, this request will update the progress indicator according to how much data it has received so far
	// The default for requests is YES
	// Also see the comments in ASINetworkQueue.h
	BOOL showAccurateProgress;
	
	// Used to ensure the progress indicator is only incremented once when showAccurateProgress = NO
	BOOL updatedProgress;
	
	// Prevents the body of the post being built more than once (largely for subclasses)
	BOOL haveBuiltPostBody;
	
	// Used internally, may reflect the size of the internal buffer used by CFNetwork
	// POST / PUT operations with body sizes greater than uploadBufferSize will not timeout unless more than uploadBufferSize bytes have been sent
	// Likely to be 32KB on iPhone 3.0, 128KB on Mac OS X Leopard and iPhone 2.2.x
	unsigned long long uploadBufferSize;
	
	// Text encoding for responses that do not send a Content-Type with a charset value. Defaults to NSISOLatin1StringEncoding
	NSStringEncoding defaultResponseEncoding;
	
	// The text encoding of the response, will be defaultResponseEncoding if the server didn't specify. Can't be set.
	NSStringEncoding responseEncoding;
	
	// Tells ASIHTTPRequest not to delete partial downloads, and allows it to use an existing file to resume a download. Defaults to NO.
	BOOL allowResumeForFileDownloads;
	
	// Custom user information associated with the request
	NSDictionary *userInfo;
	
	// Use HTTP 1.0 rather than 1.1 (defaults to false)
	BOOL useHTTPVersionOne;
	
	// When YES, requests will automatically redirect when they get a HTTP 30x header (defaults to YES)
	BOOL shouldRedirect;
	
	// Used internally to tell the main loop we need to stop and retry with a new url
	BOOL needsRedirect;
	
	// Incremented every time this request redirects. When it reaches 5, we give up
	int redirectCount;
	
	// When NO, requests will not check the secure certificate is valid (use for self-signed certificates during development, DO NOT USE IN PRODUCTION) Default is YES
	BOOL validatesSecureCertificate;
	
	// Details on the proxy to use - you could set these yourself, but it's probably best to let ASIHTTPRequest detect the system proxy settings
	NSString *proxyHost;
	int proxyPort;
	
	// ASIHTTPRequest will assume kCFProxyTypeHTTP if the proxy type could not be automatically determined
	// Set to kCFProxyTypeSOCKS if you are manually configuring a SOCKS proxy
	NSString *proxyType;

	// URL for a PAC (Proxy Auto Configuration) file. If you want to set this yourself, it's probably best if you use a local file
	NSURL *PACurl;
	
	// See ASIAuthenticationState values above. 0 == default == No authentication needed yet
	ASIAuthenticationState authenticationNeeded;
	
	// When YES, ASIHTTPRequests will present credentials from the session store for requests to the same server before being asked for them
	// This avoids an extra round trip for requests after authentication has succeeded, which is much for efficient for authenticated requests with large bodies, or on slower connections
	// Set to NO to only present credentials when explicitly asked for them
	// This only affects credentials stored in the session cache when useSessionPersistence is YES. Credentials from the keychain are never presented unless the server asks for them
	// Default is YES
	BOOL shouldPresentCredentialsBeforeChallenge;
	
	// YES when the request hasn't finished yet. Will still be YES even if the request isn't doing anything (eg it's waiting for delegate authentication). READ-ONLY
	BOOL inProgress;
	
	// Used internally to track whether the stream is scheduled on the run loop or not
	// Bandwidth throttling can unschedule the stream to slow things down while a request is in progress
	BOOL readStreamIsScheduled;
	
	// Set to allow a request to automatically retry itself on timeout
	// Default is zero - timeout will stop the request
	int numberOfTimesToRetryOnTimeout;

	// The number of times this request has retried (when numberOfTimesToRetryOnTimeout > 0)
	int retryCount;
	
	// When YES, requests will keep the connection to the server alive for a while to allow subsequent requests to re-use it for a substantial speed-boost
	// Persistent connections will not be used if the server explicitly closes the connection
	// Default is YES
	BOOL shouldAttemptPersistentConnection;

	// Number of seconds to keep an inactive persistent connection open on the client side
	// Default is 60
	// If we get a keep-alive header, this is this value is replaced with how long the server told us to keep the connection around
	// A future date is created from this and used for expiring the connection, this is stored in connectionInfo's expires value
	NSTimeInterval persistentConnectionTimeoutSeconds;
	
	// Set to yes when an appropriate keep-alive header is found
	BOOL connectionCanBeReused;
	
	// Stores information about the persistent connection that is currently in use.
	// It may contain:
	// * The id we set for a particular connection, incremented every time we want to specify that we need a new connection
	// * The date that connection should expire
	// * A host, port and scheme for the connection. These are used to determine whether that connection can be reused by a subsequent request (all must match the new request)
	// * An id for the request that is currently using the connection. This is used for determining if a connection is available or not (we store a number rather than a reference to the request so we don't need to hang onto a request until the connection expires)
	// * A reference to the stream that is currently using the connection. This is necessary because we need to keep the old stream open until we've opened a new one.
	//   The stream will be closed + released either when another request comes to use the connection, or when the timer fires to tell the connection to expire
	NSMutableDictionary *connectionInfo;
	
	// When set to YES, 301 and 302 automatic redirects will use the original method and and body, according to the HTTP 1.1 standard
	// Default is NO (to follow the behaviour of most browsers)
	BOOL shouldUseRFC2616RedirectBehaviour;
	
	// Used internally to record when a request has finished downloading data
	BOOL downloadComplete;
	
	// An ID that uniquely identifies this request - primarily used for debugging persistent connections
	NSNumber *requestID;
	
	// Will be ASIHTTPRequestRunLoopMode for synchronous requests, NSDefaultRunLoopMode for all other requests
	NSString *runLoopMode;
	
	// This timer checks up on the request every 0.25 seconds, and updates progress
	NSTimer *statusTimer;

	
	// The download cache that will be used for this request (use [ASIHTTPRequest setDefaultCache:cache] to configure a default cache
	id <ASICacheDelegate> downloadCache;
	
	// The cache policy that will be used for this request - See ASICacheDelegate.h for possible values
	ASICachePolicy cachePolicy;
	
	// The cache storage policy that will be used for this request - See ASICacheDelegate.h for possible values
	ASICacheStoragePolicy cacheStoragePolicy;
	
	// Will be true when the response was pulled from the cache rather than downloaded
	BOOL didUseCachedResponse;

	// Set secondsToCache to use a custom time interval for expiring the response when it is stored in a cache
	NSTimeInterval secondsToCache;
	
	// When downloading a gzipped response, the request will use this helper object to inflate the response
	ASIDataDecompressor *dataDecompressor;
	
	// Controls how responses with a gzipped encoding are inflated (decompressed)
	// When set to YES (This is the default):
	// * gzipped responses for requests without a downloadDestinationPath will be inflated only when [request responseData] / [request responseString] is called
	// * gzipped responses for requests with a downloadDestinationPath set will be inflated only when the request completes
	//
	// When set to NO
	// All requests will inflate the response as it comes in
	// * If the request has no downloadDestinationPath set, the raw (compressed) response is disgarded and rawResponseData will contain the decompressed response
	// * If the request has a downloadDestinationPath, the raw response will be stored in temporaryFileDownloadPath as normal, the inflated response will be stored in temporaryUncompressedDataDownloadPath
	//   Once the request completes suceessfully, the contents of temporaryUncompressedDataDownloadPath are moved into downloadDestinationPath
	//
	// Setting this to NO may be especially useful for users using ASIHTTPRequest in conjunction with a streaming parser, as it will allow partial gzipped responses to be inflated and passed on to the parser while the request is still running
	BOOL shouldWaitToInflateCompressedResponses;
	
}

#pragma mark init / dealloc

// Should be an HTTP or HTTPS url, may include username and password if appropriate
- (id)initWithURL:(NSURL *)newURL;

// Convenience constructor
+ (id)requestWithURL:(NSURL *)newURL;

+ (id)requestWithURL:(NSURL *)newURL usingCache:(id <ASICacheDelegate>)cache;
+ (id)requestWithURL:(NSURL *)newURL usingCache:(id <ASICacheDelegate>)cache andCachePolicy:(ASICachePolicy)policy;

#pragma mark setup request

// Add a custom header to the request
- (void)addRequestHeader:(NSString *)header value:(NSString *)value;

// Called during buildRequestHeaders and after a redirect to create a cookie header from request cookies and the global store
- (void)applyCookieHeader;

// Populate the request headers dictionary. Called before a request is started, or by a HEAD request that needs to borrow them
- (void)buildRequestHeaders;

// Used to apply authorization header to a request before it is sent (when shouldPresentCredentialsBeforeChallenge is YES)
- (void)applyAuthorizationHeader;


// Create the post body
- (void)buildPostBody;

// Called to add data to the post body. Will append to postBody when shouldStreamPostDataFromDisk is false, or write to postBodyWriteStream when true
- (void)appendPostData:(NSData *)data;
- (void)appendPostDataFromFile:(NSString *)file;

#pragma mark get information about this request

// Returns the contents of the result as an NSString (not appropriate for binary data - used responseData instead)
- (NSString *)responseString;

// Response data, automatically uncompressed where appropriate
- (NSData *)responseData;

// Returns true if the response was gzip compressed
- (BOOL)isResponseCompressed;

#pragma mark running a request


// Run a request synchronously, and return control when the request completes or fails
- (void)startSynchronous;

// Run request in the background
- (void)startAsynchronous;



#pragma mark HEAD request

// Used by ASINetworkQueue to create a HEAD request appropriate for this request with the same headers (though you can use it yourself)
- (ASIHTTPRequest *)HEADRequest;

#pragma mark upload/download progress

// Called approximately every 0.25 seconds to update the progress delegates
- (void)updateProgressIndicators;

// Updates upload progress (notifies the queue and/or uploadProgressDelegate of this request)
- (void)updateUploadProgress;

// Updates download progress (notifies the queue and/or uploadProgressDelegate of this request)
- (void)updateDownloadProgress;

// Called when authorisation is needed, as we only find out we don't have permission to something when the upload is complete
- (void)removeUploadProgressSoFar;

// Called when we get a content-length header and shouldResetDownloadProgress is true
- (void)incrementDownloadSizeBy:(long long)length;

// Called when a request starts and shouldResetUploadProgress is true
// Also called (with a negative length) to remove the size of the underlying buffer used for uploading
- (void)incrementUploadSizeBy:(long long)length;

// Helper method for interacting with progress indicators to abstract the details of different APIS (NSProgressIndicator and UIProgressView)
+ (void)updateProgressIndicator:(id *)indicator withProgress:(unsigned long long)progress ofTotal:(unsigned long long)total;

// Helper method used for performing invocations on the main thread (used for progress)
+ (void)performSelector:(SEL)selector onTarget:(id *)target withObject:(id)object amount:(void *)amount;

#pragma mark handling request complete / failure

// Called when a request starts, lets the delegate know via didStartSelector
- (void)requestStarted;

// Called when a request receives response headers, lets the delegate know via didReceiveResponseHeadersSelector
- (void)requestReceivedResponseHeaders;

// Called when a request completes successfully, lets the delegate know via didFinishSelector
- (void)requestFinished;

// Called when a request fails, and lets the delegate know via didFailSelector
- (void)failWithError:(NSError *)theError;

// Called to retry our request when our persistent connection is closed
// Returns YES if we haven't already retried, and connection will be restarted
// Otherwise, returns NO, and nothing will happen
- (BOOL)retryUsingNewConnection;

#pragma mark parsing HTTP response headers

// Reads the response headers to find the content length, encoding, cookies for the session 
// Also initiates request redirection when shouldRedirect is true
// And works out if HTTP auth is required
- (void)readResponseHeaders;

// Attempts to set the correct encoding by looking at the Content-Type header, if this is one
- (void)parseStringEncodingFromHeaders;

#pragma mark http authentication stuff

// Apply credentials to this request
- (BOOL)applyCredentials:(NSDictionary *)newCredentials;
- (BOOL)applyProxyCredentials:(NSDictionary *)newCredentials;

// Attempt to obtain credentials for this request from the URL, username and password or keychain
- (NSMutableDictionary *)findCredentials;
- (NSMutableDictionary *)findProxyCredentials;

// Unlock (unpause) the request thread so it can resume the request
// Should be called by delegates when they have populated the authentication information after an authentication challenge
- (void)retryUsingSuppliedCredentials;

// Should be called by delegates when they wish to cancel authentication and stop
- (void)cancelAuthentication;

// Apply authentication information and resume the request after an authentication challenge
- (void)attemptToApplyCredentialsAndResume;
- (void)attemptToApplyProxyCredentialsAndResume;

// Attempt to show the built-in authentication dialog, returns YES if credentials were supplied, NO if user cancelled dialog / dialog is disabled / running on main thread
// Currently only used on iPhone OS
- (BOOL)showProxyAuthenticationDialog;
- (BOOL)showAuthenticationDialog;

// Construct a basic authentication header from the username and password supplied, and add it to the request headers
// Used when shouldPresentCredentialsBeforeChallenge is YES
- (void)addBasicAuthenticationHeaderWithUsername:(NSString *)theUsername andPassword:(NSString *)thePassword;

#pragma mark stream status handlers

// CFnetwork event handlers
- (void)handleNetworkEvent:(CFStreamEventType)type;
- (void)handleBytesAvailable;
- (void)handleStreamComplete;
- (void)handleStreamError;

#pragma mark cleanup

// Cleans up temporary files. There's normally no reason to call these yourself, they are called automatically when a request completes or fails

// Clean up the temporary file used to store the downloaded data when it comes in (if downloadDestinationPath is set)
- (BOOL)removeTemporaryDownloadFile;

// Clean up the temporary file used to store data that is inflated (decompressed) as it comes in
- (BOOL)removeTemporaryUncompressedDownloadFile;

// Clean up the temporary file used to store the request body (when shouldStreamPostDataFromDisk is YES)
- (BOOL)removeTemporaryUploadFile;

// Clean up the temporary file used to store a deflated (compressed) request body when shouldStreamPostDataFromDisk is YES
- (BOOL)removeTemporaryCompressedUploadFile;

// Remove a file on disk, returning NO and populating the passed error pointer if it fails
+ (BOOL)removeFileAtPath:(NSString *)path error:(NSError **)err;

#pragma mark persistent connections

// Get the ID of the connection this request used (only really useful in tests and debugging)
- (NSNumber *)connectionID;

// Called automatically when a request is started to clean up any persistent connections that have expired
+ (void)expirePersistentConnections;

#pragma mark default time out

+ (NSTimeInterval)defaultTimeOutSeconds;
+ (void)setDefaultTimeOutSeconds:(NSTimeInterval)newTimeOutSeconds;

#pragma mark session credentials

+ (NSMutableArray *)sessionProxyCredentialsStore;
+ (NSMutableArray *)sessionCredentialsStore;

+ (void)storeProxyAuthenticationCredentialsInSessionStore:(NSDictionary *)credentials;
+ (void)storeAuthenticationCredentialsInSessionStore:(NSDictionary *)credentials;

+ (void)removeProxyAuthenticationCredentialsFromSessionStore:(NSDictionary *)credentials;
+ (void)removeAuthenticationCredentialsFromSessionStore:(NSDictionary *)credentials;

- (NSDictionary *)findSessionProxyAuthenticationCredentials;
- (NSDictionary *)findSessionAuthenticationCredentials;

#pragma mark keychain storage

// Save credentials for this request to the keychain
- (void)saveCredentialsToKeychain:(NSDictionary *)newCredentials;

// Save credentials to the keychain
+ (void)saveCredentials:(NSURLCredential *)credentials forHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;
+ (void)saveCredentials:(NSURLCredential *)credentials forProxy:(NSString *)host port:(int)port realm:(NSString *)realm;

// Return credentials from the keychain
+ (NSURLCredential *)savedCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;
+ (NSURLCredential *)savedCredentialsForProxy:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;

// Remove credentials from the keychain
+ (void)removeCredentialsForHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm;
+ (void)removeCredentialsForProxy:(NSString *)host port:(int)port realm:(NSString *)realm;

// We keep track of any cookies we accept, so that we can remove them from the persistent store later
+ (void)setSessionCookies:(NSMutableArray *)newSessionCookies;
+ (NSMutableArray *)sessionCookies;

// Adds a cookie to our list of cookies we've accepted, checking first for an old version of the same cookie and removing that
+ (void)addSessionCookie:(NSHTTPCookie *)newCookie;

// Dump all session data (authentication and cookies)
+ (void)clearSession;

#pragma mark get user agent

// Will be used as a user agent if requests do not specify a custom user agent
// Is only used when you have specified a Bundle Display Name (CFDisplayBundleName) or Bundle Name (CFBundleName) in your plist
+ (NSString *)defaultUserAgentString;

#pragma mark proxy autoconfiguration

// Returns an array of proxies to use for a particular url, given the url of a PAC script
+ (NSArray *)proxiesForURL:(NSURL *)theURL fromPAC:(NSURL *)pacScriptURL;

#pragma mark mime-type detection

// Return the mime type for a file
+ (NSString *)mimeTypeForFileAtPath:(NSString *)path;

#pragma mark bandwidth measurement / throttling

// The maximum number of bytes ALL requests can send / receive in a second
// This is a rough figure. The actual amount used will be slightly more, this does not include HTTP headers
+ (unsigned long)maxBandwidthPerSecond;
+ (void)setMaxBandwidthPerSecond:(unsigned long)bytes;

// Get a rough average (for the last 5 seconds) of how much bandwidth is being used, in bytes
+ (unsigned long)averageBandwidthUsedPerSecond;

- (void)performThrottling;

// Will return YES is bandwidth throttling is currently in use
+ (BOOL)isBandwidthThrottled;

// Used internally to record bandwidth use, and by ASIInputStreams when uploading. It's probably best if you don't mess with this.
+ (void)incrementBandwidthUsedInLastSecond:(unsigned long)bytes;

// On iPhone, ASIHTTPRequest can automatically turn throttling on and off as the connection type changes between WWAN and WiFi

#if TARGET_OS_IPHONE
// Set to YES to automatically turn on throttling when WWAN is connected, and automatically turn it off when it isn't
+ (void)setShouldThrottleBandwidthForWWAN:(BOOL)throttle;

// Turns on throttling automatically when WWAN is connected using a custom limit, and turns it off automatically when it isn't
+ (void)throttleBandwidthForWWANUsingLimit:(unsigned long)limit;

#pragma mark reachability

// Returns YES when an iPhone OS device is connected via WWAN, false when connected via WIFI or not connected
+ (BOOL)isNetworkReachableViaWWAN;

#endif

#pragma mark queue

// Returns the shared queue
+ (NSOperationQueue *)sharedQueue;

#pragma mark cache

+ (void)setDefaultCache:(id <ASICacheDelegate>)cache;
+ (id <ASICacheDelegate>)defaultCache;

// Returns the maximum amount of data we can read as part of the current measurement period, and sleeps this thread if our allowance is used up
+ (unsigned long)maxUploadReadLength;

#pragma mark network activity

+ (BOOL)isNetworkInUse;
#if TARGET_OS_IPHONE
+ (void)setShouldUpdateNetworkActivityIndicator:(BOOL)shouldUpdate;
#endif

#pragma mark miscellany

// Used for generating Authorization header when using basic authentication when shouldPresentCredentialsBeforeChallenge is true
// And also by ASIS3Request
+ (NSString *)base64forData:(NSData *)theData;

// Returns a date from a string in RFC1123 format
+ (NSDate *)dateFromRFC1123String:(NSString *)string;

#pragma mark threading behaviour

// In the default implementation, all requests run in a single background thread
// Advanced users only: Override this method in a subclass for a different threading behaviour
// Eg: return [NSThread mainThread] to run all requests in the main thread
// Alternatively, you can create a thread on demand, or manage a pool of threads
// Threads returned by this method will need to run the runloop in default mode (eg CFRunLoopRun())
// Requests will stop the runloop when they complete
// If you have multiple requests sharing the thread you'll need to restart the runloop when this happens
+ (NSThread *)threadForRequest:(ASIHTTPRequest *)request;


#pragma mark ===

@property (retain) NSString *username;
@property (retain) NSString *password;
@property (retain) NSString *domain;

@property (retain) NSString *proxyUsername;
@property (retain) NSString *proxyPassword;
@property (retain) NSString *proxyDomain;

@property (retain) NSString *proxyHost;
@property (assign) int proxyPort;
@property (retain) NSString *proxyType;

@property (retain,setter=setURL:) NSURL *url;
@property (retain) NSURL *originalURL;
@property (assign, nonatomic) id delegate;
@property (retain, nonatomic) id queue;
@property (assign, nonatomic) id uploadProgressDelegate;
@property (assign, nonatomic) id downloadProgressDelegate;
@property (assign) BOOL useKeychainPersistence;
@property (assign) BOOL useSessionPersistence;
@property (retain) NSString *downloadDestinationPath;
@property (retain) NSString *temporaryFileDownloadPath;
@property (retain) NSString *temporaryUncompressedDataDownloadPath;
@property (assign) SEL didStartSelector;
@property (assign) SEL didReceiveResponseHeadersSelector;
@property (assign) SEL didFinishSelector;
@property (assign) SEL didFailSelector;
@property (assign) SEL didReceiveDataSelector;
@property (retain,readonly) NSString *authenticationRealm;
@property (retain,readonly) NSString *proxyAuthenticationRealm;
@property (retain) NSError *error;
@property (assign,readonly) BOOL complete;
@property (retain) NSDictionary *responseHeaders;
@property (retain) NSMutableDictionary *requestHeaders;
@property (retain) NSMutableArray *requestCookies;
@property (retain,readonly) NSArray *responseCookies;
@property (assign) BOOL useCookiePersistence;
@property (retain) NSDictionary *requestCredentials;
@property (retain) NSDictionary *proxyCredentials;
@property (assign,readonly) int responseStatusCode;
@property (retain,readonly) NSString *responseStatusMessage;
@property (retain) NSMutableData *rawResponseData;
@property (assign) NSTimeInterval timeOutSeconds;
@property (retain) NSString *requestMethod;
@property (retain) NSMutableData *postBody;
@property (assign,readonly) unsigned long long contentLength;
@property (assign) unsigned long long postLength;
@property (assign) BOOL shouldResetDownloadProgress;
@property (assign) BOOL shouldResetUploadProgress;
@property (assign) ASIHTTPRequest *mainRequest;
@property (assign) BOOL showAccurateProgress;
@property (assign,readonly) unsigned long long totalBytesRead;
@property (assign,readonly) unsigned long long totalBytesSent;
@property (assign) NSStringEncoding defaultResponseEncoding;
@property (assign,readonly) NSStringEncoding responseEncoding;
@property (assign) BOOL allowCompressedResponse;
@property (assign) BOOL allowResumeForFileDownloads;
@property (retain) NSDictionary *userInfo;
@property (retain) NSString *postBodyFilePath;
@property (assign) BOOL shouldStreamPostDataFromDisk;
@property (assign) BOOL didCreateTemporaryPostDataFile;
@property (assign) BOOL useHTTPVersionOne;
@property (assign, readonly) unsigned long long partialDownloadSize;
@property (assign) BOOL shouldRedirect;
@property (assign) BOOL validatesSecureCertificate;
@property (assign) BOOL shouldCompressRequestBody;
@property (retain) NSURL *PACurl;
@property (retain) NSString *authenticationScheme;
@property (retain) NSString *proxyAuthenticationScheme;
@property (assign) BOOL shouldPresentAuthenticationDialog;
@property (assign) BOOL shouldPresentProxyAuthenticationDialog;
@property (assign, readonly) ASIAuthenticationState authenticationNeeded;
@property (assign) BOOL shouldPresentCredentialsBeforeChallenge;
@property (assign, readonly) int authenticationRetryCount;
@property (assign, readonly) int proxyAuthenticationRetryCount;
@property (assign) BOOL haveBuiltRequestHeaders;
@property (assign, nonatomic) BOOL haveBuiltPostBody;
@property (assign, readonly) BOOL inProgress;
@property (assign) int numberOfTimesToRetryOnTimeout;
@property (assign, readonly) int retryCount;
@property (assign) BOOL shouldAttemptPersistentConnection;
@property (assign) NSTimeInterval persistentConnectionTimeoutSeconds;
@property (assign) BOOL shouldUseRFC2616RedirectBehaviour;
@property (assign, readonly) BOOL connectionCanBeReused;
@property (retain, readonly) NSNumber *requestID;
@property (assign) id <ASICacheDelegate> downloadCache;
@property (assign) ASICachePolicy cachePolicy;
@property (assign) ASICacheStoragePolicy cacheStoragePolicy;
@property (assign, readonly) BOOL didUseCachedResponse;
@property (assign) NSTimeInterval secondsToCache;
@property (retain) ASIDataDecompressor *dataDecompressor;
@property (assign) BOOL shouldWaitToInflateCompressedResponses;
@end
