#import "TableViewController.h"

@protocol MWMStreetEditorProtocol <NSObject>

- (NSString *)getStreet;
- (void)setStreet:(NSString *)street;

- (NSArray<NSString *> *)getNearbyStreets;

@end

@interface MWMStreetEditorViewController : TableViewController

@property (weak, nonatomic) id<MWMStreetEditorProtocol> delegate;

@end
