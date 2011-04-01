#ifndef olx_filesysteH
#define olx_filesysteH

#include "estrlist.h"
#include "actions.h"

#include "url.h"
#include "etime.h"
#include "estlist.h"
#include "etraverse.h"
#include "efile.h"
#include "bapp.h"
#undef GetObject

class TFSIndex;
class TFSItem;

const uint16_t 
  afs_ReadAccess  = 0x0001,
  afs_WriteAccess = 0x0002,
  afs_DeleteAccess = 0x0004,
  afs_BrowseAccess = 0x0008,
  afs_FullAccess  = afs_ReadAccess|afs_BrowseAccess|afs_WriteAccess|afs_DeleteAccess,
  afs_ReadOnlyAccess = afs_ReadAccess|afs_BrowseAccess;


class AFileSystem : public AActionHandler  {
  olxstr FBase;
  TActionQList Actions;
protected:
  TFSIndex* Index;
  uint16_t Access;
  bool volatile Break;
  virtual bool _DoDelFile(const olxstr& f) = 0;
  virtual bool _DoDelDir(const olxstr& f) = 0;
  virtual bool _DoNewDir(const olxstr& f) = 0;
  virtual bool _DoAdoptFile(const TFSItem& src)=0;
  /* forced check only affects remote systems like http, if the value is true
  the check will be performed, otherwise, unless the index is loaded, FS dependent value
  will be returned */
  virtual bool _DoesExist(const olxstr& df, bool forced_check)=0;
  virtual IInputStream* _DoOpenFile(const olxstr& src)=0;
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name) = 0;
  // handles OnBreak
  virtual bool Execute(const IEObject* Sender, const IEObject* Data=NULL)  {
    DoBreak();
    return true;
  }
public:
  AFileSystem() : 
    Break(false),
    Index(NULL),
    Access(afs_FullAccess),  
    OnProgress(Actions.New("ON_PROGRESS")),
    OnBreak(Actions.New("ON_BREAK"))
  {
    AActionHandler::SetToDelete(false);
    OnBreak.Add(this);
  }

  virtual ~AFileSystem()  {}

  // called on progress
  TActionQueue &OnProgress,
    &OnBreak;  // add this one to the higher level handler to handle breaks

  // deletes a file
  bool DelFile(const olxstr& f)  {  
    if( (Access & afs_DeleteAccess) == 0 ) 
      return false;  
    return _DoDelFile(f);
  }
  // deletes a folder
  bool DelDir(const olxstr& d)  {
    if( (Access & afs_DeleteAccess) == 0 ) 
      return false;  
    return _DoDelDir(d);
  }
  // puts a file to the file system
  bool AdoptFile(const TFSItem& src)  {
    if( (Access & afs_WriteAccess) == 0 ) 
      return false;  
    return _DoAdoptFile(src);
  }
  // creates a new folder
  bool NewDir(const olxstr& d)  {
    if( (Access & afs_WriteAccess) == 0 ) 
      return false;  
    return _DoNewDir(d);
  }
  /* checks if the file exists, forced_check is applies to http FS, where
  the check can take some time. See _DoesExist description for more details. */
  bool Exists(const olxstr& fn, bool forced_check=false)  {
    if( (Access & afs_BrowseAccess) == 0 ) 
      return false;  
    return _DoesExist(fn, forced_check);
  }
  // returns a stream for a specified stream, must be deleted
  IInputStream* OpenFile(const olxstr& src)  {
    if( (Access & afs_ReadAccess) == 0 ) 
      return NULL;  
    return _DoOpenFile(src);
  }
  bool AdoptStream(IInputStream& file, const olxstr& name) {
    if( (Access & afs_WriteAccess) == 0 ) 
      return false;  
    return _DoAdoptStream(file, name);
  }
  void RemoveAccessRight(uint16_t access)  {
    Access &= ~access;
  }
  bool HasAccess(uint16_t access) const {  return (Access & access) != 0;  }
  DefPropP(TFSIndex*, Index)
  // returns a base at which the file system is initalised
  inline const olxstr& GetBase() const  {  return FBase; }
  inline void SetBase(const olxstr& b)  {  FBase = TEFile::AddPathDelimeter(b); }

  // depends on the file system implementation
  virtual void DoBreak()  {  Break = true;  }
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TOSFileSystem: public AFileSystem  {
  TActionQList Events;
protected:
  virtual bool _DoDelFile(const olxstr& f);
  virtual bool _DoDelDir(const olxstr& f);
  virtual bool _DoNewDir(const olxstr& f);
  virtual bool _DoAdoptFile(const TFSItem& Source);
  virtual bool _DoesExist(const olxstr& df, bool);
  virtual IInputStream* _DoOpenFile(const olxstr& src);
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name);
public:
  TOSFileSystem(const olxstr& base);
  virtual ~TOSFileSystem()  {}

  TActionQueue& OnRmFile;
  TActionQueue& OnRmDir;
  TActionQueue& OnMkDir;
  TActionQueue& OnAdoptFile;
  TActionQueue& OnOpenFile;
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TFSItem: public IEObject  {
public:
  struct SkipOptions  {
    TStrList *extsToSkip,  // extenstions to skip
             *filesToSkip; // file names to skip
    SkipOptions() : extsToSkip(NULL), filesToSkip(NULL) {  }
  };
private:
  olxstr Name, Digest;
  uint64_t Size, DateTime;
  bool Folder, Processed;
  TFSItem* Parent;
  TCSTypeList<olxstr, TFSItem*> Items;
  TStrList Properties, Actions;
  TFSIndex& Index;
protected:
  void DeleteItem(TFSItem* item);
  bool IsProcessed() const {  return Processed;  }
  // recursive version, must be called with false before Syncronise or CalcDiffSize
  void SetProcessed(bool v);
public:
  TFSItem(TFSIndex& index, TFSItem* parent, const olxstr& name) :
    Index(index), 
    Parent(parent), 
    Folder(false), 
    Processed(false) ,
    DateTime(0), 
    Size(0), 
    Name(name) {  }
  virtual ~TFSItem()  {  Clear();  }
  void Clear();
  inline TFSItem* GetParent() const {  return Parent; }

  void operator >> (TStrList& strings) const;
  size_t ReadStrings(size_t& index, TFSItem* caller, TStrList& strings, const SkipOptions* toSkip=NULL);
  // removes empty folders recursively
  void ClearEmptyFolders();
  // removes nonexiting files recursively
  void ClearNonexisting();

  TFSItem& operator = (const TFSItem& FI);
  inline TFSItem& Item(size_t i) const {  return *Items.GetObject(i); }
  inline size_t Count() const {  return Items.Count(); }
  inline bool IsEmpty() const {  return Items.IsEmpty(); }
  TFSItem& NewItem(const olxstr& name);
  // recreates specified item in current context
  TFSItem& NewItem(TFSItem* item);
  // removes the item and deletes the file/folder
  static void Remove(TFSItem& item);

  inline size_t PropertyCount() const {  return Properties.Count();  }
  inline const olxstr& GetProperty(size_t ind) const {  return Properties[ind];  }
  inline void AddProperty(const olxstr& p)  {  Properties.Add(p);  }
  inline bool HasProperty(const olxstr& pn)  const {
    return Properties.IndexOf(pn) != InvalidIndex;
  }
  inline bool ValidateProperties(const TStrList& prs) const {
    if( Properties.IsEmpty() || prs.IsEmpty() )  return true;
    for( size_t i=0; i < prs.Count(); i++ )
      if( Properties.IndexOf(prs[i]) != InvalidIndex )
        return true;
    return false;
  }
  void ListUniqueProperties(TStrList& Properties);

  const TStrList& GetActions() const {  return Actions;  }

  DefPropBIsSet(Folder)

  inline const olxstr& GetName() const {  return Name;  }

  int GetLevel()  const;
  olxstr GetFullName() const;

  DefPropP(uint64_t, DateTime)
  DefPropP(uint64_t, Size)
  DefPropC(olxstr, Digest)
  // only updates the digest if current is empty
  size_t UpdateDigest();

  template <class SC> TFSItem* FindByName(const SC& Name) const {
    const size_t ind = Items.IndexOf(Name);
    return (ind == InvalidIndex) ? NULL : Items.GetObject(ind);
  }
	// does a search of /parent_folder/parent_folder/file_name
  TFSItem* FindByFullName(const olxstr& Name) const;

  AFileSystem& GetIndexFS() const;
  AFileSystem& GetDestFS() const;
  // calculates the update size
  uint64_t CalcDiffSize(TFSItem& Dest, const TStrList& properties);
  // syncronises two items
  uint64_t Synchronise(TFSItem& Dest, const TStrList& properties, TStrList* cmds=NULL);
  TFSItem* UpdateFile(TFSItem& FN);
  /* deletes underlying physical object (file or folder). If the object is a folder
  the content of that folder will be removed completely */
  void DelFile();

  uint64_t CalcTotalItemsSize(const TStrList& props) const;

  static TGraphTraverser<TFSItem> Traverser;
};
//.............................................................................//
//.............................................................................//
//.............................................................................//
class TFSIndex: public IEObject  {
private:
  TFSItem *Root;
  bool IndexLoaded;
  mutable bool Break;
protected:
  olxstr Source, Destination;
  TStrList Properties;
  TActionQList Actions;
  AFileSystem *DestFS;
  AFileSystem& IndexFS;
  TOnProgress Progress;
  TActionQueue &OnBreak;
public:
  TFSIndex(AFileSystem& fs);
  virtual ~TFSIndex();
  
  // this is to be used for the overal progress monitorring
  TActionQueue &OnProgress;
  // this is to be used for when an action is being applied to a file (like extract)
  TActionQueue &OnAction;

  void LoadIndex(const olxstr& IndexFile, const TFSItem::SkipOptions* toSkip=NULL);
  void SaveIndex(const olxstr& IndexFile);
  /* returns the number transfered bytes.  If the dest_fs is not NULL, the difference is adopted by that
  file syste. If cmds is not NULL, the rm commands are stored in it */
  uint64_t Synchronise(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip=NULL, 
    AFileSystem* dest_fs=NULL, TStrList* cmds=NULL, const olxstr& indexName="index.ind");
  uint64_t CalcDiffSize(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip=NULL,
    const olxstr& indexName="index.ind");
  // returns true if the file is updated (added) and false otherwise
  bool UpdateFile(AFileSystem& To, const olxstr& fileName, bool Force, const olxstr& indexName="index.ind");
  inline TFSItem& GetRoot()  const {  return *Root; }
  /* checks if the file actions specify to delete it, if a delete action is found return false
  if the timestamps of the items and size match and false in other cases; updates the dest digest if empty */
  bool ShallAdopt(const TFSItem& src, TFSItem& dest) const;
  bool ShouldExist(const TFSItem& src)  const {  return src.GetActions().IndexOfi("delete") == InvalidIndex;  }
  // returns if the action was procesed (or not) successful
  bool ProcessActions(TFSItem& item); 
  // stops the syncronisation and updates the index
  void DoBreak() {  
    Break = true;
    OnBreak.Execute(this);
  }
  bool IsInterrupted() const {
    return Break && Progress.GetPos() != Progress.GetMax();
  }
  friend class TFSItem;
};
#endif
