#ifndef ALLEGRO_PLUGIN_H_
#define ALLEGRO_PLUGIN_H_

#include <wx/log.h>

#include <io_mgr.h>

class ALLEGRO_PLUGIN : public PLUGIN
{
public:
    const wxString PluginName() const override { return wxS( "Cadence Allegro" ); }

    PLUGIN_FILE_DESC GetBoardFileDesc() const override
    {
        return PLUGIN_FILE_DESC( _HKI( "Cadence Allegro layout file" ), { "brd" } );
    }

    PLUGIN_FILE_DESC GetFootprintFileDesc() const override
    {
        wxLogWarning( "Func %s", __FUNCTION__ );
        return GetBoardFileDesc();
    }

    PLUGIN_FILE_DESC GetFootprintLibDesc() const override
    {
        wxLogWarning( "Func? %s", __FUNCTION__ );
        return GetBoardFileDesc();
    }

    long long GetLibraryTimestamp( const wxString& aLibraryPath ) const override
    {
        wxLogWarning( "Func %s", __FUNCTION__ );
        // No support for libraries....
        return 1;
    }

    bool                    CanReadBoard( const wxString& aFileName ) const override;
    bool                    CanReadFootprint( const wxString& aFileName ) const override;
    bool                    CanReadFootprintLib( const wxString& aFileName ) const override;
    std::vector<FOOTPRINT*> GetImportedCachedLibraryFootprints() override
    {
        wxLogWarning( "Func %s", __FUNCTION__ );
        std::vector<FOOTPRINT*> fps;
        return fps;
    }

    BOARD* LoadBoard( const wxString& aFileName, BOARD* aAppendToMe,
                      const STRING_UTF8_MAP* aProperties = nullptr, PROJECT* aProject = nullptr,
                      PROGRESS_REPORTER* aProgressReporter = nullptr ) override;

    void FootprintEnumerate( wxArrayString& aFootprintNames, const wxString& aLibraryPath,
                             bool aBestEfforts, const STRING_UTF8_MAP* aProperties ) override
    {
        wxLogWarning( "Func %s", __FUNCTION__ );
    }

    void FootprintLibCreate( const wxString&        aLibraryPath,
                             const STRING_UTF8_MAP* aProperties ) override
    {
        wxLogWarning( "Func %s", __FUNCTION__ );
    }

    bool IsFootprintLibWritable( const wxString& aLibraryPath ) override { return false; }

    ALLEGRO_PLUGIN();
    ~ALLEGRO_PLUGIN() override;

private:
    BOARD* m_board;
};

#endif // ALLEGRO_PLUGIN_H_
