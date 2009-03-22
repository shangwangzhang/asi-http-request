//
//  ASINetworkQueueTests.m
//  asi-http-request
//
//  Created by Ben Copsey on 08/11/2008.
//  Copyright 2008 All-Seeing Interactive. All rights reserved.
//

#import "ASINetworkQueueTests.h"
#import "ASIHTTPRequest.h"
#import "ASINetworkQueue.h"

@implementation ASINetworkQueueTests


- (void)testProgress
{
	complete = NO;
	progress = 0;
	
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:NO];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
	ASIHTTPRequest *request1 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request1];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/trailsnetwork.png"] autorelease];
	ASIHTTPRequest *request2 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request2];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/sharedspace20.png"] autorelease];
	ASIHTTPRequest *request3 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request3];
	
	[networkQueue go];
		
	 while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	 }
	
	BOOL success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
	//Now test again with accurate progress
	[networkQueue cancelAllOperations];
	[networkQueue setShowAccurateProgress:YES];

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/logo.png"] autorelease];
	request1 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request1];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/i/trailsnetwork.png"] autorelease];
	request2 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request2];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/sharedspace20.png"] autorelease];
	request3 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request3];
	
	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];
	
	// Progress maths are inexact for queues
	success = (progress > 0.95);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
	
	[networkQueue release];
	
	
	
	
}



- (void)setProgress:(float)newProgress
{
	progress = newProgress;
}



- (void)testFailure
{
	complete = NO;
	
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(requestFailed:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];
	[networkQueue setShouldCancelAllRequestsOnFailure:NO];
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	ASIHTTPRequest *request1 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request1];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/second"] autorelease];
	ASIHTTPRequest *request2 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request2];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/third"] autorelease];
	ASIHTTPRequest *request3 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request3];
	
	url = [[[NSURL alloc] initWithString:@""] autorelease];
	requestThatShouldFail = [[ASIHTTPRequest alloc] initWithURL:url];
	[networkQueue addOperation:requestThatShouldFail];

	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/broken"] autorelease];
	ASIHTTPRequest *request5 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request5];

	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];
	
	
	BOOL success;
	success = ([request1 error] == nil);
	GHAssertTrue(success,@"Request 1 failed");
	
	success = [[request1 responseString] isEqualToString:@"This is the expected content for the first string"];
	GHAssertTrue(success,@"Failed to download the correct data for request 1");
	
	success = ([request2 error] == nil);
	GHAssertTrue(success,@"Request 2 failed");
	
	success = [[request2 responseString] isEqualToString:@"This is the expected content for the second string"];
	GHAssertTrue(success,@"Failed to download the correct data for request 2");
	
	success = ([request3 error] == nil);
	GHAssertTrue(success,@"Request 3 failed");
	
	success = [[request3 responseString] isEqualToString:@"This is the expected content for the third string"];
	GHAssertTrue(success,@"Failed to download the correct data for request 3");
	
	success = ([requestThatShouldFail error] != nil);
	GHAssertTrue(success,@"Request 4 succeed when it should have failed");
	
	success = ([request5 error] == nil);
	GHAssertTrue(success,@"Request 5 failed");
	
	success = ([request5 responseStatusCode] == 404);
	GHAssertTrue(success,@"Failed to obtain the correct status code for request 5");


	
	[requestThatShouldFail release];
	
}


- (void)testFailureCancelsOtherRequests
{
	complete = NO;
	
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDelegate:self];
	[networkQueue setRequestDidFailSelector:@selector(requestFailedCancellingOthers:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/first"] autorelease];
	ASIHTTPRequest *request1 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request1];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/second"] autorelease];
	ASIHTTPRequest *request2 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request2];
	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/third"] autorelease];
	ASIHTTPRequest *request3 = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request3];
	
	url = [[[NSURL alloc] initWithString:@""] autorelease];
	requestThatShouldFail = [[ASIHTTPRequest alloc] initWithURL:url];
	[networkQueue addOperation:requestThatShouldFail];

	[networkQueue go];
	
	[networkQueue waitUntilAllOperationsAreFinished];
	
	
	[requestThatShouldFail release];	
}

 
- (void)requestFailedCancellingOthers:(ASIHTTPRequest *)request
{
	complete = YES;
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	BOOL success = (request == requestThatShouldFail);
	GHAssertTrue(success,@"Wrong request failed");
}

- (void)queueFinished:(ASINetworkQueue *)queue
{
	complete = YES;
}



- (void)testProgressWithAuthentication
{
	complete = NO;
	progress = 0;
	
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	[networkQueue setRequestDidFailSelector:@selector(requestFailedCancellingOthers:)];
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com/ASIHTTPRequest/tests/basic-authentication"] autorelease];
	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[networkQueue addOperation:request];
	
	[networkQueue go];
	

	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	}

	NSError *error = [request error];
	GHAssertNotNil(error,@"The HEAD request failed, but it didn't tell the main request to fail");	
	[networkQueue release];
	
	
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	request = [[[ASIHTTPRequest alloc] initWithURL:url] autorelease];
	[request setUsername:@"secret_username"];
	[request setPassword:@"secret_password"];
	[networkQueue addOperation:request];
	
	[networkQueue go];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	}
	
	error = [request error];
	GHAssertNil(error,@"Failed to use authentication in a queue");	
	[networkQueue release];
	
}



- (void)requestFailedExpectedly:(ASIHTTPRequest *)request
{
    request_didfail = YES;
    BOOL success = (request == requestThatShouldFail);
    GHAssertTrue(success,@"Wrong request failed");
}

- (void)requestSucceededUnexpectedly:(ASIHTTPRequest *)request
{
    request_succeeded = YES;
}

//Connect to a port the server isn't listening on, and the read stream won't be created (Test + Fix contributed by Michael Krause)
- (void)testWithNoListener
{
    request_succeeded = NO;
    request_didfail = NO;
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
    [networkQueue setRequestDidFailSelector:@selector(requestFailedExpectedly:)];
    [networkQueue setRequestDidFinishSelector:@selector(requestSucceededUnexpectedly:)];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	NSURL *url;	
	url = [[[NSURL alloc] initWithString:@"http://allseeing-i.com:9999/i/logo.png"] autorelease];
	requestThatShouldFail = [[ASIHTTPRequest alloc] initWithURL:url];
	[networkQueue addOperation:requestThatShouldFail];
	
	[networkQueue go];
	[networkQueue waitUntilAllOperationsAreFinished];
    
	GHAssertTrue(!request_succeeded && request_didfail,@"Request to resource without listener succeeded but should have failed");
    
	[networkQueue release];
}

- (void)testPartialResume
{
	complete = NO;
	progress = 0;
	
	NSString *temporaryPath = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"MemexTrails_1.0b1.zip.download"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:temporaryPath]) {
		[[NSFileManager defaultManager] removeItemAtPath:temporaryPath error:nil];
	}
	
	NSString *downloadPath = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"MemexTrails_1.0b1.zip"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:downloadPath]) {
		[[NSFileManager defaultManager] removeItemAtPath:downloadPath error:nil];
	}
	
	NSURL *downloadURL = [NSURL URLWithString:@"http://trails-network.net/Downloads/MemexTrails_1.0b1.zip"];
	networkQueue = [[ASINetworkQueue alloc] init];	

	ASIHTTPRequest *request = [[[ASIHTTPRequest alloc] initWithURL:downloadURL] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setAllowResumeForFileDownloads:YES];
	[networkQueue addOperation:request];
	[networkQueue go];
	 
	// Let the download run for 5 seconds, which hopefully won't be enough time to grab this file. If you have a super fast connection, this test may fail, serves you right for being so smug. :)
	NSTimer *timeoutTimer = [NSTimer scheduledTimerWithTimeInterval:5 target:self selector:@selector(stopQueue:) userInfo:nil repeats:NO];
	
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	}
	
	// 5 seconds is up, let's tell the queue to stop
	[networkQueue cancelAllOperations];
	
	[networkQueue release];
	networkQueue = [[ASINetworkQueue alloc] init];
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	complete = NO;
	progress = 0;	
	unsigned long long downloadedSoFar = [[[NSFileManager defaultManager] fileAttributesAtPath:temporaryPath traverseLink:NO] fileSize];
	BOOL success = (downloadedSoFar > 0);
	GHAssertTrue(success,@"Failed to download part of the file, so we can't proceed with this test");
	
	request = [[[ASIHTTPRequest alloc] initWithURL:downloadURL] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setAllowResumeForFileDownloads:YES];
	
	[networkQueue addOperation:request];

	[networkQueue go];

	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	}
	
	unsigned long long amountDownloaded = [[[NSFileManager defaultManager] fileAttributesAtPath:downloadPath traverseLink:NO] fileSize];
	success = (amountDownloaded == 9145357);
	GHAssertTrue(success,@"Failed to complete the download");
	
	success = (progress == 1.0);
	GHAssertTrue(success,@"Failed to increment progress properly");
	
	[networkQueue release];	
	
	
	//Test the temporary file cleanup
	complete = NO;
	progress = 0;
	networkQueue = [[ASINetworkQueue alloc] init];	
	[networkQueue setDownloadProgressDelegate:self];
	[networkQueue setShowAccurateProgress:YES];
	[networkQueue setDelegate:self];
	[networkQueue setQueueDidFinishSelector:@selector(queueFinished:)];	
	
	request = [[[ASIHTTPRequest alloc] initWithURL:downloadURL] autorelease];
	[request setDownloadDestinationPath:downloadPath];
	[request setTemporaryFileDownloadPath:temporaryPath];
	[request setAllowResumeForFileDownloads:YES];
	[networkQueue addOperation:request];
	[networkQueue go];
	
	// Let the download run for 5 seconds
	timeoutTimer = [NSTimer scheduledTimerWithTimeInterval:5 target:self selector:@selector(stopQueue:) userInfo:nil repeats:NO];
	while (!complete) {
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
	}
	[networkQueue cancelAllOperations];
	
	success = ([[NSFileManager defaultManager] fileExistsAtPath:temporaryPath]);
	GHAssertTrue(success,@"Temporary download file doesn't exist");	
	
	[request removeTemporaryDownloadFile];
	
	success = (![[NSFileManager defaultManager] fileExistsAtPath:temporaryPath]);
	GHAssertTrue(success,@"Temporary download file should have been deleted");		
	
	timeoutTimer = nil;
	[networkQueue release];	
	
}

- (void)stopQueue:(id)sender
{
	complete = YES;
}


 
@end
