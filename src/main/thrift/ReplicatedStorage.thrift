namespace cpp mipt.distsys.storage
namespace java mipt.distsys.storage

struct ViewInfo {
  1: i32 view,
  2: string primary,
  3: string backup,
}

exception ServiceException {
  1: i32 what,
  2: string why,
}

const i32 DEAD_PINGS = 3;

service Coordinator {
  ViewInfo ping(1:i32 viewNum, 2:string name) throws (1:ServiceException e),
  string primary() throws (1:ServiceException e),
  i32 tick(),
}
