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

exception IncorrectOperationException {
  1: i32 what,
  2: string why,
}

const i32 DEAD_PINGS = 3;

service Coordinator {
  ViewInfo ping(1:i32 viewNum, 2:string name) throws (1:ServiceException e),
  string primary() throws (1:ServiceException e),
  i32 tick(),
}

service Server {
  void put(1:string key, 2:string value) throws (1:IncorrectOperationException e),
  void putBackup(1:string key, 2:string value) throws (1:IncorrectOperationException e),
  string get(1:string key) throws (1:IncorrectOperationException e),
  i32 tick(),

  void setCoordinator(1:string hostPort),
}
