/*
 *  cPopulation.h
 *  Avida
 *
 *  Called "population.hh" prior to 12/5/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef cPopulation_h
#define cPopulation_h

#include "avida/data/Provider.h"

#include "cBirthChamber.h"
#include "cOrgInterface.h"
#include "cPopulationInterface.h"
#include "cPopulationResources.h"
#include "cResourcePopulationInterface.h"
#include "cString.h"
#include "cWorld.h"
#include "tList.h"

#include <fstream>
#include <map>

class cAvidaContext;
class cCodeLabel;
class cEnvironment;
class cLineage;
class cOrganism;
class cPopulationCell;

using namespace Avida;


class cPopulationOrgStatProvider : public Data::ArgumentedProvider
{
public:
  ~cPopulationOrgStatProvider();

  virtual void UpdateReset() = 0;
  virtual void HandleOrganism(cOrganism* org) = 0;
};

typedef Apto::SmartPtr<cPopulationOrgStatProvider, Apto::InternalRCObject> cPopulationOrgStatProviderPtr;


class cPopulation : public Data::ArgumentedProvider, public cResourcePopulationInterface
{
private:
  // Components...
  cWorld* m_world;
  cPopulationResources m_pop_res;           // Resources available
  Apto::PriorityScheduler* m_scheduler;                // Handles allocation of CPU cycles
  Apto::Array<cPopulationCell> cell_array;  // Local cells composing the population
  Apto::Array<int> empty_cell_id_array;     // Used for PREFER_EMPTY birth methods
  cBirthChamber birth_chamber;         // Global birth chamber.
  //Keeps track of which organisms are in which group.
  Apto::Map<int, Apto::Array<cOrganism*, Apto::Smart> > m_group_list;
  Apto::Map<int, Apto::Array<pair<int,int> > > m_group_intolerances;
  Apto::Map<int, Apto::Array<pair<int,int> > > m_group_intolerances_females;
  Apto::Map<int, Apto::Array<pair<int,int> > > m_group_intolerances_males;
  Apto::Map<int, Apto::Array<pair<int,int> > > m_group_intolerances_juvs;
  
  // Keep list of live organisms
  Apto::Array<cOrganism*, Apto::Smart> live_org_list;
  
  Apto::Array<cPopulationOrgStatProviderPtr> m_org_stat_providers;
  
  
  // Data Tracking...
  tList<cPopulationCell> reaper_queue; // Death order in some mass-action runs
  Apto::Array<int, Apto::Smart> minitrace_queue;
  bool print_mini_trace_genomes;
  bool print_mini_trace_reacs;
  bool use_micro_traces;
  int m_next_prey_q;
  int m_next_pred_q;
  
  Apto::Array<cOrganism*, Apto::Smart> repro_q;
  Apto::Array<cOrganism*, Apto::Smart> topnav_q;
  
  // Default organism setups...
  cEnvironment& environment;          // Physics & Chemistry description

  // Other data...
  int world_x;                         // Structured population width.
  int world_y;                         // Structured population height.
  int num_organisms;                   // Cell count with living organisms
  int num_prey_organisms;
  int num_pred_organisms;
  int num_top_pred_organisms;
  
  // Outside interactions...
  bool sync_events;   // Do we need to sync up the event list with population?
	
  // Group formation information
  std::map<int, int> m_groups; //<! Maps the group id to the number of orgs in the group
  std::map<int, int> m_group_females; //<! Maps the group id to the number of females in the group
  std::map<int, int> m_group_males; //<! Maps the group id to the number of males in the group

  cPopulation(); // @not_implemented
  cPopulation(const cPopulation&); // @not_implemented
  cPopulation& operator=(const cPopulation&); // @not_implemented
  
  
public:
  cPopulation(cWorld* world);
  ~cPopulation();


  cPopulationResources& GetResources() { return m_pop_res; }
  
  // Data::Provider
  Data::ConstDataSetPtr Provides() const;
  void UpdateProvidedValues(Update current_update);
  Apto::String DescribeProvidedValue(const Apto::String& data_id) const;
  bool SupportsConcurrentUpdate() const;
  
  // Data::ArgumentedProvider
  void SetActiveArguments(const Data::DataID& data_id, Data::ConstArgumentSetPtr args);
  Data::ConstArgumentSetPtr GetValidArguments(const Data::DataID& data_id) const;
  bool IsValidArgument(const Data::DataID& data_id, Data::Argument arg) const;
  
  Data::PackagePtr GetProvidedValueForArgument(const Data::DataID& data_id, const Data::Argument& arg) const;

  // cPopulation
  
  void AttachOrgStatProvider(cPopulationOrgStatProviderPtr provider) { m_org_stat_providers.Push(provider); }
  
  void ResizeCellGrid(int x, int y);
    
  void InjectGenome(int cell_id, Systematics::Source src, const Genome& genome, cAvidaContext& ctx, int lineage_label = 0, bool assign_group = true, Systematics::RoleClassificationHints* hints = NULL);

  // Activate the offspring of an organism in the population
  bool ActivateOffspring(cAvidaContext& ctx, const Genome& offspring_genome, cOrganism* parent_organism);
  bool ActivateParasite(cOrganism* host, Systematics::UnitPtr parent, const cString& label, const InstructionSequence& injected_code);
  
  // Helper function for ActivateParasite - returns if the parasite from the infected host should infect the target host
  bool TestForParasiteInteraction(cOrganism* infected_host, cOrganism* target_host);
  
  void UpdateQs(cOrganism* parent, bool reproduced = false);
  
  // Inject an organism from the outside world.
  void Inject(const Genome& genome, Systematics::Source src, cAvidaContext& ctx, int cell_id = -1, double merit = -1, int lineage_label = 0, double neutral_metric = 0, bool inject_with_group = false, int group_id = -1, int forager_type = -1, int trace = 0); 
  void InjectGroup(const Genome& genome, Systematics::Source src, cAvidaContext& ctx, int cell_id = -1, double merit = -1, int lineage_label = 0, double neutral_metric = 0, int group_id = -1, int forager_type = -1, int trace = 0);
  void InjectParasite(const cString& label, const InstructionSequence& injected_code, int cell_id);
  
  // Deactivate an organism in the population (required for deactivations)
  void KillOrganism(cPopulationCell& in_cell, cAvidaContext& ctx); 
  void KillOrganism(cAvidaContext& ctx, int in_cell) { KillOrganism(cell_array[in_cell], ctx); } 
  void InjureOrg(cPopulationCell& in_cell, double injury);
  
  // @WRE 2007/07/05 Helper function to take care of side effects of Avidian
  // movement that cannot be directly handled in cHardwareCPU.cc
  bool MoveOrganisms(cAvidaContext& ctx, int src_cell_id, int dest_cell_id, int avatar_cell);

  // Specialized functionality
  void Kaboom(cPopulationCell& in_cell, cAvidaContext& ctx, int distance=0); 
  void SwapCells(int cell_id1, int cell_id2, cAvidaContext& ctx); 

  
  // Print donation stats
  void PrintDonationStats();

  // Process a single organism one instruction...
  int ScheduleOrganism();          // Determine next organism to be processed.
  void ProcessStep(cAvidaContext& ctx, double step_size, int cell_id);
  void ProcessStepSpeculative(cAvidaContext& ctx, double step_size, int cell_id);

  // Calculate the statistics from the most recent update.
  void ProcessPostUpdate(cAvidaContext& ctx);
  void ProcessPreUpdate();
  void ProcessUpdateCellActions(cAvidaContext& ctx);

  // Clear all but a subset of cells...
  void SerialTransfer(int transfer_size, bool ignore_deads, cAvidaContext& ctx); 

  // Saving and loading...
  bool SavePopulation(const cString& filename, bool save_historic, bool save_group_info = false, bool save_avatars = false,
                      bool save_rebirth = false);
  bool SaveStructuredSystematicsGroup(const Systematics::RoleID& role, const cString& filename);
  bool LoadStructuredSystematicsGroup(cAvidaContext& ctx, const Systematics::RoleID& role, const cString& filename);
  bool LoadPopulation(const cString& filename, cAvidaContext& ctx, int cellid_offset=0, int lineage_offset=0,
                      bool load_groups = false, bool load_birth_cells = false, bool load_avatars = false, bool load_rebirth = false, bool load_parent_dat = false);
  bool SaveFlameData(const cString& filename);
  
  void SetMiniTraceQueue(Apto::Array<int, Apto::Smart> new_queue, const bool print_genomes, const bool print_reacs, const bool use_micro = false);
  void AppendMiniTraces(Apto::Array<int, Apto::Smart> new_queue, const bool print_genomes, const bool print_reacs, const bool use_micro = false);
  void LoadMiniTraceQ(cString& filename, int orgs_per, bool print_genomes, bool print_reacs);
  Apto::Array<int, Apto::Smart> SetRandomTraceQ(int max_samples);
  Apto::Array<int, Apto::Smart> SetRandomPreyTraceQ(int max_samples);
  Apto::Array<int, Apto::Smart> SetRandomPredTraceQ(int max_samples);
  void SetNextPreyQ(int num_prey, bool print_genomes, bool print_reacs, bool use_micro);
  void SetNextPredQ(int num_pred, bool print_genomes, bool print_reacs, bool use_micro);
  Apto::Array<int, Apto::Smart> SetTraceQ(int save_dominants, int save_groups, int save_foragers, int orgs_per, int max_samples);
  const Apto::Array<int, Apto::Smart>& GetMiniTraceQueue() const { return minitrace_queue; }
  void AppendRecordReproQ(cOrganism* new_org);
  void SetTopNavQ();
  Apto::Array<cOrganism*, Apto::Smart>& GetTopNavQ() { return topnav_q; }
  
  int GetSize() const { return cell_array.GetSize(); }
  int GetWorldX() const { return world_x; }
  int GetWorldY() const { return world_y; }

  cPopulationCell& GetCell(int in_num) { return cell_array[in_num]; }

  cBirthChamber& GetBirthChamber(int id) { (void) id; return birth_chamber; }

  void ResetInputs(cAvidaContext& ctx);

  cEnvironment& GetEnvironment() { return environment; }
  int GetNumOrganisms() { return num_organisms; }
  int GetNumPreyOrganisms() { return num_prey_organisms; }
  int GetNumPredOrganisms() { return num_pred_organisms; }
  int GetNumTopPredOrganisms() { return num_top_pred_organisms; }

  void DecNumPreyOrganisms() { num_prey_organisms--; }
  void DecNumPredOrganisms() { num_pred_organisms--; }
  void DecNumTopPredOrganisms() { num_top_pred_organisms--; }

  void IncNumPreyOrganisms() { num_prey_organisms++; }
  void IncNumPredOrganisms() { num_pred_organisms++; }
  void IncNumTopPredOrganisms() { num_top_pred_organisms++; }
  
  void RemovePredators(cAvidaContext& ctx);
  void InjectPreyClone(cAvidaContext& ctx, cOrganism* org_to_clone);
  
  bool GetSyncEvents() { return sync_events; }
  void SetSyncEvents(bool _in) { sync_events = _in; }
  void PrintPhenotypeData(const cString& filename);
  void PrintHostPhenotypeData(const cString& filename);
  void PrintParasitePhenotypeData(const cString& filename);
  void PrintPhenotypeStatus(const cString& filename);

  bool UpdateMerit(int cell_id, double new_merit);

  // Trials and genetic algorithm @JEB
  void NewTrial(cAvidaContext& ctx);
  void CompeteOrganisms(cAvidaContext& ctx, int competition_type, int parents_survive);
  
  // Add an org to live org list
  void AddLiveOrg(cOrganism* org);  
  // Remove an org from live org list
  void RemoveLiveOrg(cOrganism* org); 
  const Apto::Array<cOrganism*, Apto::Smart>& GetLiveOrgList() const { return live_org_list; }
	
  // Adds an organism to a group  
  void JoinGroup(cOrganism* org, int group_id);
  void MakeGroup(cOrganism* org);
  // Removes an organism from a group 
  void LeaveGroup(cOrganism* org, int group_id);

  //Kill random member of the group (but not self!!!) 
  void KillGroupMember(cAvidaContext& ctx, int group_id, cOrganism* org);
  void AttackFacedOrg(cAvidaContext& ctx, int loser);
  void KillRandPred(cAvidaContext& ctx, cOrganism* org);
  void KillRandPrey(cAvidaContext& ctx, cOrganism* org);
  // Identifies the number of organisms in a group
  int NumberOfOrganismsInGroup(int group_id);
  int NumberGroupFemales(int group_id);
  int NumberGroupMales(int group_id);
  int NumberGroupJuvs(int group_id);
  void ChangeGroupMatingTypes(cOrganism* org, int group_id, int old_type, int new_type);
  // Get the group information
  map<int, int> GetFormedGroups() { return m_groups; }

  // -------- Tolerance support --------
  int CalcGroupToleranceImmigrants(int group_id, int mating_type = -1);
  int CalcGroupToleranceOffspring(cOrganism* parent_organism);
  double CalcGroupOddsImmigrants(int group_id, int mating_type  = -1);
  bool AttemptImmigrateGroup(cAvidaContext& ctx, int group_id, cOrganism* org);
  double CalcGroupOddsOffspring(int group_id);
  double CalcGroupOddsOffspring(cOrganism* parent);
  bool AttemptOffspringParentGroup(cAvidaContext& ctx, cOrganism* parent, cOrganism* offspring);
  double CalcGroupAveImmigrants(int group_id, int mating_type = -1);
  double CalcGroupSDevImmigrants(int group_id, int mating_type = -1);
  double CalcGroupAveOwn(int group_id);
  double CalcGroupSDevOwn(int group_id);
  double CalcGroupAveOthers(int group_id);
  double CalcGroupSDevOthers(int group_id);
  int& GetGroupIntolerances(int group_id, int tol_num, int mating_type);

  // -------- Population mixing support --------
  //! Mix all organisms in the population.
  void MixPopulation(cAvidaContext& ctx);

private:
  void SetupCellGrid();
  void ClearCellGrid();
  void BuildTimeSlicer(); // Build the schedule object
  
  // Methods to place offspring in the population.
  cPopulationCell& PositionOffspring(cPopulationCell& parent_cell, cAvidaContext& ctx, bool parent_ok = true); 
  void PositionAge(cPopulationCell& parent_cell, tList<cPopulationCell>& found_list, bool parent_ok);
  void PositionMerit(cPopulationCell & parent_cell, tList<cPopulationCell>& found_list, bool parent_ok);
  Apto::Array<int>& GetEmptyCellIDArray() { return empty_cell_id_array; }
  void FindEmptyCell(tList<cPopulationCell>& cell_list, tList<cPopulationCell>& found_list);
  int FindRandEmptyCell(cAvidaContext& ctx);
  
  // Update statistics collecting...
  void UpdateOrganismStats(cAvidaContext& ctx);
  void UpdateFTOrgStats(cAvidaContext& ctx); 
  void UpdateMaleFemaleOrgStats(cAvidaContext& ctx);
  
  void InjectClone(int cell_id, cOrganism& orig_org, Systematics::Source src);
  void CompeteOrganisms_ConstructOffspring(int cell_id, cOrganism& parent);
  
  void CCladeSetupOrganism(cOrganism* organism);
	
  // Must be called to activate *any* organism in the population.
  bool ActivateOrganism(cAvidaContext& ctx, cOrganism* in_organism, cPopulationCell& target_cell, bool assign_group = true, bool is_inject = false);
  
  void TestForMiniTrace(cOrganism* in_organism);
  void SetupMiniTrace(cOrganism* in_organism);
  void PrintMiniTraceGenome(cOrganism* in_organism, cString& filename);
  
  int PlaceAvatar(cAvidaContext& ctx, cOrganism* parent);
  
  inline void AdjustSchedule(const cPopulationCell& cell, const cMerit& merit);
};

#endif
