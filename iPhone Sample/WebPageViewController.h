//
//  WebPageViewController.h
//  iPhone
//
//  Created by Ben Copsey on 03/10/2010.
//  Copyright 2010 All-Seeing Interactive. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SampleViewController.h"
#import <WebKit/WebKit.h>

@class ASIWebPageRequest;

@interface WebPageViewController : SampleViewController <WKNavigationDelegate> {
	WKWebView *webView;
	UITextField *urlField;
	UITextView *responseField;
	UISwitch *replaceURLsSwitch;
	ASIWebPageRequest *request;
	NSMutableArray *requestsInProgress;
}
- (void)fetchURL:(NSURL *)url;

@property (retain, nonatomic) ASIWebPageRequest *request;
@property (retain, nonatomic) NSMutableArray *requestsInProgress;
@end
