#ifndef PODIO_DATASOURCE_H
#define PODIO_DATASOURCE_H

// Podio
#include <podio/CollectionBase.h>
#include <podio/Frame.h>
#include <podio/Reader.h>

// ROOT
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RDataSource.hxx>

// STL
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

namespace podio {
class DataSource : public ROOT::RDF::RDataSource {
public:
  ///
  /// @brief Construct the podio::DataSource from the provided file.
  ///
  /// @param filePath Path to the file that should be read
  /// @param nEvents Number of events to process (optional, defaults to -1 for
  ///                all events)
  /// @param collsToRead The collections that should be made available (optional,
  ///                    defaults to empty vector for all collections)
  ///
  explicit DataSource(const std::string& filePath, int nEvents = -1, const std::vector<std::string>& collsToRead = {});

  ///
  /// @brief Construct the podio::DataSource from the provided file list.
  ///
  /// @param filePathList Paths to the files that should be read
  /// @param nEvents Number of events to process (optional, defaults to -1 for
  ///                all events)
  /// @param collsToRead The collections that should be made available (optional,
  ///                    defaults to empty vector for all collections)
  ///
  explicit DataSource(const std::vector<std::string>& filePathList, int nEvents = -1,
                      const std::vector<std::string>& collsToRead = {});

  ///
  /// @brief Inform the podio::DataSource of the desired level of parallelism.
  ///
  void SetNSlots(unsigned int nSlots) override;

  ///
  /// @brief Inform podio::DataSource that an event-loop is about to start.
  ///
  void Initialize() override;

  ///
  /// @brief Retrieve from podio::DataSource a set of ranges of entries that
  ///        can be processed concurrently.
  ///
  std::vector<std::pair<ULong64_t, ULong64_t>> GetEntryRanges() override;

  ///
  /// @brief Inform podio::DataSource that a certain thread is about to start
  ///        working on a certain range of entries.
  ///
  void InitSlot(unsigned int slot, ULong64_t firstEntry) override;

  ///
  /// @brief Inform podio::DataSource that a certain thread is about to start
  ///        working on a certain entry.
  ///
  bool SetEntry(unsigned int slot, ULong64_t entry) override;

  ///
  /// @brief Inform podio::DataSource that a certain thread finished working
  ///        on a certain range of entries.
  ///
  void FinalizeSlot(unsigned int slot) override;

  ///
  /// @brief Inform podio::DataSource that an event-loop finished.
  ///
  void Finalize() override;

  ///
  /// @brief Returns a reference to the collection of the dataset's column
  ///        names
  ///
  const std::vector<std::string>& GetColumnNames() const override;

  ///
  /// @brief Checks if the dataset has a certain column.
  ///
  bool HasColumn(std::string_view columnName) const override;

  ///
  /// @brief Type of a column as a string. Required for JITting.
  ///
  std::string GetTypeName(std::string_view columnName) const override;

  std::string GetLabel() override {
    return "PODIO Datasource";
  };

protected:
  ///
  /// @brief Type-erased vector of pointers to pointers to column
  ///        values --- one per slot.
  ///
  std::vector<void*> GetColumnReadersImpl(std::string_view name, const std::type_info& typeInfo) override;

  std::string AsString() override {
    return "Podio data source";
  }

private:
  /// Number of slots/threads
  unsigned int m_nSlots = 1;

  /// Input filename
  std::vector<std::string> m_filePathList = {};

  /// Total number of events
  ULong64_t m_nEvents = 0;

  /// Ranges of events available to be processed
  std::vector<std::pair<ULong64_t, ULong64_t>> m_rangesAvailable = {};

  /// Ranges of events available ever created
  std::vector<std::pair<ULong64_t, ULong64_t>> m_rangesAll = {};

  /// Column names
  std::vector<std::string> m_columnNames{};

  /// Column types
  std::vector<std::string> m_columnTypes = {};

  /// Collections, m_Collections[columnIndex][slotIndex]
  std::vector<std::vector<const podio::CollectionBase*>> m_Collections = {};

  /// Active collections
  std::vector<unsigned int> m_activeCollections = {};

  /// Root podio readers
  std::vector<std::unique_ptr<podio::Reader>> m_podioReaders = {};

  /// Podio frames
  std::vector<std::unique_ptr<podio::Frame>> m_frames = {};

  ///
  /// @brief Setup input for the podio::DataSource.
  ///
  /// @param[in] nEvents Number of events.
  ///
  void SetupInput(int nEvents, const std::vector<std::string>& collsToRead);
};

///
/// @brief Create RDataFrame from multiple Podio files.
///
/// @param[in] filePathList  List of file paths from which the RDataFrame
///                          will be created.
/// @param[in] collsToRead   List of collection names that should be made
///                          available
///
/// @return                  RDataFrame created from input file list.
///
ROOT::RDataFrame CreateDataFrame(const std::vector<std::string>& filePathList,
                                 const std::vector<std::string>& collsToRead = {});

///
/// @brief Create RDataFrame from a Podio file or glob pattern matching multiple Podio files.
///
/// @param[in] filePath  File path from which the RDataFrame will be created.
///                      The file path can include glob patterns to match multiple files.
/// @param[in] collsToRead   List of collection names that should be made
///                          available
///
/// @return              RDataFrame created from input file list.
///
ROOT::RDataFrame CreateDataFrame(const std::string& filePath, const std::vector<std::string>& collsToRead = {});
} // namespace podio

#endif /* PODIO_DATASOURCE_H */
