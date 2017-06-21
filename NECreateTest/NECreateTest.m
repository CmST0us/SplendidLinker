//
//  NECreateTest.m
//  NECreateTest
//
//  Created by CmST0us on 17/5/31.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <NetworkExtension/NetworkExtension.h>

@interface NECreateTest : XCTestCase

@end

@implementation NECreateTest

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testCreateProfile {
    NETunnelProviderManager *manager = [[NETunnelProviderManager alloc] init];
    NETunnelProviderProtocol *protocol = [[NETunnelProviderProtocol alloc] init];
    protocol.providerBundleIdentifier = @"com.CmST0us.SplendidLinker.PacketTunnel";
    protocol.username = @"CmST0us";
    protocol.serverAddress = @"127.0.0.1";
    protocol.disconnectOnSleep = NO;
    manager.protocolConfiguration = protocol;
    [manager saveToPreferencesWithCompletionHandler:^(NSError * _Nullable error) {
        if (error != NULL) {
            NSLog(@"ERROR %@", error);
        } else {
            NSLog(@"Save OK");
        }
    }];
    [[NSRunLoop currentRunLoop] run];
}

- (void)testStart {
    [NETunnelProviderManager loadAllFromPreferencesWithCompletionHandler:^(NSArray<NETunnelProviderManager *> * _Nullable managers, NSError * _Nullable error) {
        [managers enumerateObjectsUsingBlock:^(NETunnelProviderManager * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
            
            NETunnelProviderSession *session = (NETunnelProviderSession *)obj.connection;
            NSError *error;
            [session startTunnelWithOptions:@{@"key":@"value"} andReturnError:&error];
            if (error != nil) {
                //        NEVPNError
                NSLog(@"%@", error);
            } else {
                NSLog(@"OK");
            }

        }];
    }];
    
    [[NSRunLoop currentRunLoop] run];
}
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
