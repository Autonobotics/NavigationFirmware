#include <windows.h> 
#include <stdio.h>


#define BUFFERSIZE ( 1024 )

BYTE Buffer[ BUFFERSIZE ] ;


int main( int argc, char *argv[] )
{
  char *PipeName ;
  HANDLE PipeHandle ;
  BOOL Connected, Success ;
  DWORD BytesRead ;

  PipeHandle = INVALID_HANDLE_VALUE ;
  PipeName = "\\\\.\\pipe\\myitmpipe" ;

  PipeHandle = CreateNamedPipe( 
          PipeName,             // pipe name 
          PIPE_ACCESS_DUPLEX,
          PIPE_TYPE_BYTE |
          PIPE_READMODE_BYTE |
          PIPE_WAIT,                // blocking mode 
          1, // max. instances  
          BUFFERSIZE,                  // output buffer size 
          BUFFERSIZE,                  // input buffer size 
          0,                        // client time-out 
          NULL);                    // default security attribute 

  if( INVALID_HANDLE_VALUE != PipeHandle )
  {  /* pipe handle ok */
    printf( "pipe handle ok\n" ) ;
    Connected = ConnectNamedPipe( PipeHandle, NULL ) ;
    if( 0 != Connected )
    {  /* client connected */
      printf( "client connected\n" ) ;
      while( Connected )
      {
        Success = ReadFile( PipeHandle, Buffer, 2, &BytesRead, NULL ) ;
        if( 0 != Success )
        {
          Buffer[ BytesRead ] = '\0' ;
          printf( "%s", Buffer ) ;
        }
        else
        {
          switch( GetLastError( ) )
          {
            case ERROR_IO_PENDING:  /* no real problem */
              break ;
            default:  /* some error */
              Connected = 0 ;
              break ;
          }
        }
      }
      printf( "shutting down\n" ) ;
      DisconnectNamedPipe( PipeHandle ) ;
    }
    else
    {  /* client connection failed */ 
      printf( "client connection failed\n" ) ;
    }
    CloseHandle( PipeHandle ) ;
  }
  else
  {  /* pipe handle not ok */
    printf( "pipe handle not ok\n" ) ;
  }

  return( 0 ) ;
} ;
