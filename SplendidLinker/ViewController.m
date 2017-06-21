//
//  ViewController.m
//  SplendidLinker
//
//  Created by CmST0us on 17/5/31.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#import "ViewController.h"
#import <NetworkExtension/NetworkExtension.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
//    
    
//
    
    [NETunnelProviderManager loadAllFromPreferencesWithCompletionHandler:^(NSArray<NETunnelProviderManager *> * _Nullable managers, NSError * _Nullable error) {
        NETunnelProviderManager *manager = [[NETunnelProviderManager alloc] init];
        if ([managers count] == 0) {
            NETunnelProviderProtocol *protocol = [[NETunnelProviderProtocol alloc] init];
            protocol.providerBundleIdentifier = @"com.CmST0us.SplendidLinker.PacketTunnel";
            protocol.username = @"CmST0us";
            protocol.serverAddress = @"192.168.2.193:10080";
            protocol.disconnectOnSleep = NO;
            manager.protocolConfiguration = protocol;
            
            [manager saveToPreferencesWithCompletionHandler:^(NSError * _Nullable error) {
                if (error != NULL) {
                    NSLog(@"ERROR %@", error);
                } else {
                    NSLog(@"Save OK");
                }
            }];
        }
        manager = [managers firstObject];
        if (manager == nil) {
            return ;
        }
        NETunnelProviderSession *session = (NETunnelProviderSession *)manager.connection;
        NSError *err;
//        [session startTunnelWithOptions:nil andReturnError:&err];
        NSLog(@"ALL");
        
    }];
    
    
//    [session sendProviderMessage:[@"message" dataUsingEncoding:NSUTF8StringEncoding] returnError:&error responseHandler:^(NSData * _Nullable responseData) {
//        NSString *s = [[NSString alloc] initWithData:responseData encoding:NSUTF8StringEncoding];
//        NSLog(@"%@", s);
//    }];
    
//    [session startTunnelWithOptions:@{@"key":@"value"} andReturnError:&error];
//    if (error != nil) {
//        NSLog(@"%@", error);
//    } else {
//        NSLog(@"OK");
//    }
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
