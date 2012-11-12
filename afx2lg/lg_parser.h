#pragma once
#ifndef LG_PARSER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "preset.h"

// merge the tr1 namespace into std for convenience.
namespace std {
  using namespace tr1;
}

struct LgParserCallback {
  virtual const PresetMap& GetPresetMap() = 0;
  virtual void WriteLine(const char* line, int length) = 0;
};

class LgParser {
 public:
  class LgEntry {
   public:
    typedef std::vector<std::string> Lines;

    LgEntry() {}
    virtual ~LgEntry() {}

    virtual void AppendLine(const char* line, const char* eol);
    virtual void WriteLines(LgParserCallback* callback);

    const Lines& lines() const { return lines_; }

   protected:
    Lines lines_;
  };

  class NamedEntry : public LgEntry {
   public:
    NamedEntry() {}
    ~NamedEntry() {}

    const std::string& name() const { return name_; }
    virtual void SetName(const std::string& name);

    virtual void AppendLine(const char* line, const char* eol);

   protected:
    std::string name_;
  };

  class BankList : public NamedEntry {
   public:
    BankList() {}
    virtual ~BankList() {}

    virtual void AppendLine(const char* line, const char* eol);
    virtual void WriteLines(LgParserCallback* callback);

    void AppendBank(const std::string& bank_name);

   private:
  };

  class Bank : public NamedEntry {
   public:
    Bank() {}
    virtual ~Bank() {}

    virtual void AppendLine(const char* line, const char* eol);
    virtual void WriteLines(LgParserCallback* callback);
    void SetBankList(const std::shared_ptr<BankList>& bank_list);
    void OnPatchNameChange(const std::string& old_name,
                           const std::string& new_name);
    std::vector<std::string> GetPatchNames() const;
    void SetInheritedFrom(const std::string& bank_name);
    void RemoveNonPatchEntries();

    const std::shared_ptr<BankList>& bank_list() { return bank_list_; }

   private:
    std::shared_ptr<BankList> bank_list_;
    std::string inherited_from_name_;
    std::string default_preset_;
  };

  class Patch : public NamedEntry {
   public:
    Patch()
       : channel_(1), preset_(0), bank_id_(0), cc_index_(-1), pc_index_(-1) {}
    virtual ~Patch() {}
    virtual void AppendLine(const char* line, const char* eol);
    virtual void SetName(const std::string& name);
    void Update(const Preset& p);
    void SetPreset(int preset_number);
    void SetBank(const std::shared_ptr<Bank>& bank);

    int preset() const;
    const std::shared_ptr<Bank> bank() const { return bank_; }

   private:
    int channel_;
    int preset_;
    int bank_id_;
    int cc_index_;
    int pc_index_;
    std::shared_ptr<Bank> bank_;
  };

  LgParser();
  ~LgParser();

  void ParseBuffer(LgParserCallback* callback, const char* begin,
      const char* end);

 protected:
  typedef std::vector<std::shared_ptr<LgEntry> > Entries;
  typedef std::vector<std::shared_ptr<Patch> > Patches;
  typedef std::vector<std::shared_ptr<Bank> > Banks;
  typedef std::vector<std::shared_ptr<BankList> > BankLists;

  void ProcessLine(const char* line, const char* end);
  std::shared_ptr<LgEntry> CreateEntry(const char* line);
  void ConnectBanksToBankLists();
  void ConnectPatchesToBanks();
  Patches::value_type LookupPatch(const std::string& name);
  Patches::value_type LookupPatch(int preset_id);
  Entries::iterator GetPatchInsertPoint();

 private:
  Entries entries_;
  std::shared_ptr<LgEntry> current_entry_;

  Patches patches_;
  Banks banks_;
  BankLists bank_lists_;
};

#endif  // LG_PARSER_H_