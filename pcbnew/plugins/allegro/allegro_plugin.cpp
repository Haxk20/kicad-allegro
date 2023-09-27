#include <wx/wfstream.h>
#include <wx/log.h>
#include <wx/translation.h>

#include <board.h>

#include "allegro_file.h"
#include "allegro_pcb.h"
#include "allegro_plugin.h"
#include "allegro_structs.h"

ALLEGRO_PLUGIN::ALLEGRO_PLUGIN() : m_board( nullptr ){};

ALLEGRO_PLUGIN::~ALLEGRO_PLUGIN() = default;

bool ALLEGRO_PLUGIN::CanReadBoard( const wxString& aFileName ) const
{
    if( !PLUGIN::CanReadBoard( aFileName ) )
        return false;

    wxFileInputStream input( aFileName );
    if( input.IsOk() && !input.Eof() )
    {
        uint32_t magic = 0;
        if( input.GetLength() < sizeof( magic ) )
        {
            return false;
        }

        if( !input.ReadAll( &magic, sizeof( magic ) ) )
        {
            return false;
        }

        if( ALLEGRO::A_160 <= magic && magic <= ALLEGRO::A_MAX )
        {
            return true;
        }
    }

    return false;
};

BOARD* ALLEGRO_PLUGIN::LoadBoard( const wxString& aFileName, BOARD* aAppendToMe,
                                  const STRING_UTF8_MAP* aProperties, PROJECT* aProject,
                                  PROGRESS_REPORTER* aProgressReporter )
{
    // FIXME: What is the intent of appending to a board?
    m_board = aAppendToMe ? aAppendToMe : new BOARD();

    if( !aAppendToMe )
    {
        m_board->SetFileName( aFileName );
    }

    ALLEGRO_FILE allegroBrdFile( aFileName );
    ALLEGRO_PCB  pcb( m_board, aProgressReporter );
    pcb.Parse( allegroBrdFile );

    return m_board;
}
