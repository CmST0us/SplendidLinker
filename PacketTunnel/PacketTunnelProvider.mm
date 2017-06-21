//
//  PacketTunnelProvider.m
//  PacketTunnel
//
//  Created by CmST0us on 17/5/31.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#import <SplendidTunnel/SLClient.hpp>
#import <SplendidTunnel/TunnelMessage.pb.h>
#import <functional>
#import "PacketTunnelProvider.h"
#include <thread>
typedef void(^TunnelCompletionBlock)(NSError *);

@interface PacketTunnelProvider () {
    SLClient *_clientTunnel;
    
    TunnelCompletionBlock _completionBlock;
    
}

- (void)tunnelDidEstablish:(st::TunnelMessage &)msg;
- (void)tunnelDidHaveError;
- (void)tunnelDidReadPacket:(st::TunnelMessage &)msg;
@end


#pragma mark - callback


#warning TODO: CTX 上下文错误
static void cb_error(SLTunnel::TunnelStatus s, void *ctx) {
//    PacketTunnelProvider *provider = (__bridge PacketTunnelProvider *)ctx;
    
}

static void cb_establish(SLTunnel::TunnelStatus s, st::TunnelMessage & msg, void *ctx) {
    PacketTunnelProvider *provider = (__bridge PacketTunnelProvider *)ctx;
    [provider tunnelDidEstablish:msg];
}

static void cb_read(SLTunnel::TunnelStatus s, st::TunnelMessage & msg, void *ctx) {
    PacketTunnelProvider *provider = (__bridge PacketTunnelProvider *)ctx;
    [provider tunnelDidReadPacket:msg];
}

@implementation PacketTunnelProvider

- (void)startTunnelWithOptions:(NSDictionary *)options completionHandler:(void (^)(NSError *))completionHandler {
	// Add code here to start the process of connecting the tunnel.
    
    _clientTunnel = new SLClient((__bridge void*)self);
#warning TODO: 需要设计
    _clientTunnel->get_mOptions()[OPTIONS_USERNAME_KEY] = "admin";
    _clientTunnel->get_mOptions()[OPTIONS_PASSWORD_KEY] = "admin";
    
    _clientTunnel->set_mErrorCallback(cb_error).set_mEstablishCallback(cb_establish).set_mReadPacketCallback(cb_read);
    
    
    NSArray *ta = [self.protocolConfiguration.serverAddress componentsSeparatedByString:@":"];
    NSString *addressString = ta[0];
    NSString *portString = ta[1];
    NSLog(@"addressString: %@", addressString);
    NSLog(@"portString: %@", portString);
    auto status = _clientTunnel->startTunnelWithServerAddress([addressString cStringUsingEncoding:NSASCIIStringEncoding], [portString integerValue]);
    std::thread t([&](){
        _clientTunnel->dispatchEvent();
    });
    t.detach();
    switch (status) {
        case SLTunnel::OnEstablish:
            _completionBlock = completionHandler;
            break;
        case SLTunnel::ConfigureInvalid:
            NSLog(@"Splendid On Invalid!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            completionHandler([NSError errorWithDomain:@"SL" code:1 userInfo:NULL]);
            NSLog(@"Splendid Here!!!!!!!!!!!!!!!!!!!!!!!!");
            break;
            
        case SLTunnel::ServerUnReachable:
            NSLog(@"Splendid On readchable!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            completionHandler([NSError errorWithDomain:@"SL" code:2 userInfo:NULL]);
            NSLog(@"Splendid Here!!!!!!!!!!!!!!!!!!!!!!!!");
            break;
        case SLTunnel::TunnelInitError:
            NSLog(@"Splendid On readchable!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            completionHandler([NSError errorWithDomain:@"SL" code:3 userInfo:NULL]);
            NSLog(@"Splendid Here!!!!!!!!!!!!!!!!!!!!!!!!");
            break;
        default:
            break;
    }
    
}

- (void)stopTunnelWithReason:(NEProviderStopReason)reason completionHandler:(void (^)(void))completionHandler {
	// Add code here to start the process of stopping the tunnel.
	completionHandler();
    exit(0);
}

- (void)handleAppMessage:(NSData *)messageData completionHandler:(void (^)(NSData *))completionHandler {
	// Add code here to handle the message.
    completionHandler(messageData);
}

- (void)sleepWithCompletionHandler:(void (^)(void))completionHandler {
	// Add code here to get ready to sleep.
	completionHandler();
}

- (void)wake {
	// Add code here to wake up.
    NSLog(@"Wake Up");
}


#pragma mark - callback method

//func createTunnelSettingsFromConfiguration(configuration: [NSObject: AnyObject]) -> NEPacketTunnelNetworkSettings? {
//    guard let tunnelAddress = tunnel?.remoteHost,
//    address = getValueFromPlist(configuration, keyArray: [.IPv4, .Address]) as? String,
//    netmask = getValueFromPlist(configuration, keyArray: [.IPv4, .Netmask]) as? String
//    else { return nil }
//    
//    let newSettings = NEPacketTunnelNetworkSettings(tunnelRemoteAddress: tunnelAddress)
//    var fullTunnel = true
//    
//    newSettings.IPv4Settings = NEIPv4Settings(addresses: [address], subnetMasks: [netmask])
//    
//    if let routes = getValueFromPlist(configuration, keyArray: [.IPv4, .Routes]) as? [[String: AnyObject]] {
//        var includedRoutes = [NEIPv4Route]()
//        for route in routes {
//            if let netAddress = route[SettingsKey.Address.rawValue] as? String,
//                netMask = route[SettingsKey.Netmask.rawValue] as? String
//            {
//                includedRoutes.append(NEIPv4Route(destinationAddress: netAddress, subnetMask: netMask))
//            }
//        }
//        newSettings.IPv4Settings?.includedRoutes = includedRoutes
//        fullTunnel = false
//    }
//    else {
//        // No routes specified, use the default route.
//        newSettings.IPv4Settings?.includedRoutes = [NEIPv4Route.defaultRoute()]
//    }
//    
//    if let DNSDictionary = configuration[SettingsKey.DNS.rawValue] as? [String: AnyObject],
//        DNSServers = DNSDictionary[SettingsKey.Servers.rawValue] as? [String]
//    {
//        newSettings.DNSSettings = NEDNSSettings(servers: DNSServers)
//        if let DNSSearchDomains = DNSDictionary[SettingsKey.SearchDomains.rawValue] as? [String] {
//            newSettings.DNSSettings?.searchDomains = DNSSearchDomains
//            if !fullTunnel {
//                newSettings.DNSSettings?.matchDomains = DNSSearchDomains
//            }
//        }
//    }
//    
//    newSettings.tunnelOverheadBytes = 150
//    
//    return newSettings
//}

- (void)handlePacket:(NSArray *)a
                   b:(NSArray *)b{
    [a enumerateObjectsUsingBlock:^(NSData * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        st::TunnelMessage msg;
        msg.set_type(st::TunnelMessage_Type_REQUEST);
        msg.set_messagecode(st::TunnelMessage_MessageCode_DATA);
        msg.mutable_packet()->set_packetdata(obj.bytes, obj.length);
        
        uint32_t l = msg.ByteSize();
        std::unique_ptr<unsigned char> b(new unsigned char[l]);
        msg.SerializeToArray(b.get(), l);
        
        _clientTunnel->writePacket(b.get(), l);
    }];
    
    [self.packetFlow readPacketsWithCompletionHandler:^(NSArray<NSData *> * _Nonnull packets, NSArray<NSNumber *> * _Nonnull protocols) {
        [self handlePacket:packets b:protocols];
    }];
}

- (void)startHandlePacket {
    [self.packetFlow readPacketsWithCompletionHandler:^(NSArray<NSData *> * _Nonnull packets, NSArray<NSNumber *> * _Nonnull protocols) {
        [self handlePacket:packets b:protocols];
    }];
}
- (void)tunnelDidEstablish:(st::TunnelMessage &)msg {
    self->_completionBlock(nil);
    
    NSString *address = [[NSString alloc] initWithBytes:msg.ip().data() length:msg.ip().size() encoding:NSUTF8StringEncoding];
    NSString *submask = [[NSString alloc] initWithBytes:msg.submask().data() length:msg.submask().size() encoding:NSUTF8StringEncoding];
    NEPacketTunnelNetworkSettings *setting = [[NEPacketTunnelNetworkSettings alloc] initWithTunnelRemoteAddress:@"192.168.2.193"];
    setting.IPv4Settings = [[NEIPv4Settings alloc] initWithAddresses:@[address] subnetMasks:@[@"255.255.255.255"]];
    NEIPv4Route *route = [[NEIPv4Route alloc] initWithDestinationAddress:@"0.0.0.0" subnetMask:@"0.0.0.0"];
    setting.IPv4Settings.includedRoutes = @[route];
    setting.DNSSettings = [[NEDNSSettings alloc] initWithServers:@[@"8.8.8.8"]];
    
    [self setTunnelNetworkSettings:setting completionHandler:^(NSError * _Nullable error) {
        NSLog(@"设置网络 %@", error);
        
    }];
    [self startHandlePacket];
}

- (void)tunnelDidHaveError {
    
}

- (void)tunnelDidReadPacket:(st::TunnelMessage &)msg {
    //向本机tun写入
    NSData *d = [[NSData alloc] initWithBytes:msg.packet().packetdata().data() length:msg.packet().packetdata().size()];
    [self.packetFlow writePackets:@[d] withProtocols:@[[NSNumber numberWithInt:PF_INET]]];
}

@end


