//
//  FirstViewController.m
//  webp
//
//  Created by Alex on 2016/12/20.
//  Copyright © 2016年 Alex. All rights reserved.
//

#import "FirstViewController.h"
#import "UIImage+WebP.h"

static CGFloat quality = 75.0f;
static CGFloat alpha = 0.6f;
static BOOL asyncConvert = YES;
@interface FirstViewController ()
@property (weak, nonatomic) IBOutlet UIImageView *normalView;
@property (weak, nonatomic) IBOutlet UIImageView *convertedView;

@end

@implementation FirstViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    UIImage *demoImage = [UIImage imageNamed:@"MN"];
    [_normalView setImage:[demoImage imageByApplyingAlpha:alpha]];
    
    NSData *demoImageData = UIImageJPEGRepresentation(demoImage, 1.0);
    uint64_t fileSize = [demoImageData length];
    
    
    [_convertedView setImage:[UIImage imageNamed:@"Default"]];
    
    if (!asyncConvert) {
        NSData *webPData = [UIImage imageToWebP:demoImage quality:quality];
        [self displayImageWithData:webPData];
    }
    else {
        [UIImage imageToWebP:demoImage quality:quality alpha:alpha preset:WEBP_PRESET_PHOTO completionBlock:^(NSData *result) {
            [self displayImageWithData:result];
        } failureBlock:^(NSError *error) {
            NSLog(@"%@", error.localizedDescription);
        }];
    }

    // Do any additional setup after loading the view, typically from a nib.
}

- (void)displayImageWithData:(NSData *)webPData
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *webPPath = [paths[0] stringByAppendingPathComponent:@"image.webp"];
    
    if ([webPData writeToFile:webPPath atomically:YES]) {
        uint64_t fileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:webPPath error:nil] fileSize];
       
        
        if (!asyncConvert) {
            [_convertedView setImage:[UIImage imageWithWebP:webPPath]];
        }
        else {
            [UIImage imageWithWebP:webPPath completionBlock:^(UIImage *result) {
                [_convertedView setImage:result];
            }failureBlock:^(NSError *error) {
                NSLog(@"%@", error.localizedDescription);
            }];
        }
    }
}
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}



#pragma mark - Helper

- (NSString *)contentTypeForImageData:(NSData *)data {
    uint8_t c;
    [data getBytes:&c length:1];
    
    switch (c) {
        case 0xFF:
            return @"JPEG";
        case 0x89:
            return @"PNG";
        case 0x47:
            return @"GIF";
        case 0x49:
            break;
        case 0x42:
            return @"BMP";
        case 0x4D:
            return @"TIFF";
    }
    
    return nil;
}


@end
