/*
 *  cWorld.h
 *  Avida
 *
 *  Created by David on 10/18/05.
 *  Copyright 2005 Michigan State University. All rights reserved.
 *
 */

#ifndef cWorld_h
#define cWorld_h

#ifndef cAvidaConfig_h
#include "cAvidaConfig.h"
#endif
#ifndef cDataFileManager_h
#include "cDataFileManager.h"
#endif
#ifndef cRandom_h
#include "cRandom.h"
#endif

class cEnvironment;
class cEventManager;
class cEventList;
class cHardwareManager;
class cPopulation;
class cStats;
class cTestCPU;

class cWorld
{
protected:
  cAvidaConfig* m_conf;
  cDataFileManager* m_data_mgr;
  cEnvironment* m_env;
  cEventManager* m_event_mgr;
  cEventList* m_event_list;
  cHardwareManager* m_hw_mgr;
  cPopulation* m_pop;
  cStats* m_stats;
  cTestCPU* m_test_cpu;

  cRandom m_rng;
  
  bool m_test_on_div;
  bool m_test_sterilize;

  void Setup();

public:
  explicit cWorld() : m_conf(new cAvidaConfig()) { Setup(); }
  cWorld(cAvidaConfig* cfg) : m_conf(cfg) { Setup(); }
  ~cWorld();
  
  void SetConfig(cAvidaConfig* cfg) { delete m_conf; m_conf = cfg; }
  
  // General Object Accessors
  cAvidaConfig& GetConfig() { return *m_conf; }
  cDataFileManager& GetDataFileManager() { return *m_data_mgr; }
  cEnvironment& GetEnvironment() { return *m_env; }
  cHardwareManager& GetHardwareManager() { return *m_hw_mgr; }
  cPopulation& GetPopulation() { return *m_pop; }
  cRandom& GetRandom() { return m_rng; }
  cStats& GetStats() { return *m_stats; }
  cTestCPU& GetTestCPU() { return *m_test_cpu; }
  
  // Access to Data File Manager
  std::ofstream& GetDataFileOFStream(const cString& fname) { return m_data_mgr->GetOFStream(fname); }
  cDataFile& GetDataFile(const cString& fname) { return m_data_mgr->Get(fname); }  

  // Config Dependent Modes
  bool GetTestOnDivide() const { return m_test_on_div; }
  bool GetTestSterilize() const { return m_test_sterilize; }
  
  // Convenience Accessors
  int GetNumInstructions();
  int GetNumTasks();
  int GetNumReactions();
  int GetNumResources();

  // DDD - Inherited from cAvidaDriver heritage
  void GetEvents();
  void ReadEventListFile(const cString & filename);
  void SyncEventList();
};

#endif
