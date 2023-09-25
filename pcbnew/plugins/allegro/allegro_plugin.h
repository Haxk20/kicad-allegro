#ifndef ALLEGRO_PLUGIN_H_
#define ALLEGRO_PLUGIN_H_

#include <io_mgr.h>

class ALLEGRO_PLUGIN : public PLUGIN
{
public:
    const wxString PluginName() const override { return wxS( "Cadence Allegro" ); }

    PLUGIN_FILE_DESC GetBoardFileDesc() const override
    {
        return PLUGIN_FILE_DESC( _HKI( "Cadence Allegro layout file" ), { "brd" } );
    }


    long long GetLibraryTimestamp( const wxString& aLibraryPath ) const override
    {
        // No support for libraries....
        return 0;
    }

    bool CanReadBoard( const wxString& aFileName ) const override;

    BOARD* LoadBoard( const wxString& aFileName, BOARD* aAppendToMe,
                      const STRING_UTF8_MAP* aProperties = nullptr, PROJECT* aProject = nullptr,
                      PROGRESS_REPORTER* aProgressReporter = nullptr ) override;

    ALLEGRO_PLUGIN();
    ~ALLEGRO_PLUGIN() override;

private:
    BOARD* m_board;
};

#endif // ALLEGRO_PLUGIN_H_
