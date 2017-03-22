// Copyright 2016 Intel Corporation All Rights Reserved
// 
// Intel makes no representations about the suitability of this software for any purpose.
// THIS SOFTWARE IS PROVIDED ""AS IS."" INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES,
// EXPRESS OR IMPLIED, AND ALL LIABILITY, INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES,
// FOR THE USE OF THIS SOFTWARE, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY
// RIGHTS, AND INCLUDING THE WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// Intel does not assume any responsibility for any errors which may appear in this software
// nor any responsibility to update it.

#include <thread>
#include <chrono>
#include "OperatingSystem.h"

namespace ApiWithoutSecrets {

  namespace OS {

    Window::Window() :
      Parameters() {
    }

    WindowParameters Window::GetParameters() const {
      return Parameters;
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)

#define TUTORIAL_NAME "API without Secrets: Introduction to Vulkan"

    LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
      switch( message ) {
      case WM_SIZE:
      case WM_EXITSIZEMOVE:
        PostMessage( hWnd, WM_USER + 1, wParam, lParam );
        break;
      case WM_KEYDOWN:
      case WM_CLOSE:
        PostMessage( hWnd, WM_USER + 2, wParam, lParam );
        break;
      default:
        return DefWindowProc( hWnd, message, wParam, lParam );
      }
      return 0;
    }

    Window::~Window() {
      if( Parameters.Handle ) {
        DestroyWindow( Parameters.Handle );
      }

      if( Parameters.Instance ) {
        UnregisterClass( TUTORIAL_NAME, Parameters.Instance );
      }
    }

    bool Window::Create( const char *title ) {
      Parameters.Instance = GetModuleHandle( nullptr );

      // Register window class
      WNDCLASSEX wcex;

      wcex.cbSize = sizeof(WNDCLASSEX);

      wcex.style = CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc = WndProc;
      wcex.cbClsExtra = 0;
      wcex.cbWndExtra = 0;
      wcex.hInstance = Parameters.Instance;
      wcex.hIcon = NULL;
      wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
      wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      wcex.lpszMenuName = NULL;
      wcex.lpszClassName = TUTORIAL_NAME;
      wcex.hIconSm = NULL;

      if( !RegisterClassEx( &wcex ) ) {
        return false;
      }

      // Create window
      Parameters.Handle = CreateWindow( TUTORIAL_NAME, title, WS_OVERLAPPEDWINDOW, 20, 20, 500, 500, nullptr, nullptr, Parameters.Instance, nullptr );
      if( !Parameters.Handle ) {
        return false;
      }

      return true;
    }

    bool Window::RenderingLoop( TutorialBase &tutorial ) const {
      // Display window
      ShowWindow( Parameters.Handle, SW_SHOWNORMAL );
      UpdateWindow( Parameters.Handle );

      // Main message loop
      MSG message;
      bool loop = true;
      bool resize = false;
      bool result = true;

      while( loop ) {
        if( PeekMessage( &message, NULL, 0, 0, PM_REMOVE ) ) {
          // Process events
          switch( message.message ) {
            // Resize
          case WM_USER + 1:
            resize = true;
            break;
            // Close
          case WM_USER + 2:
            loop = false;
            break;
          }
          TranslateMessage( &message );
          DispatchMessage( &message );
        } else {
          // Draw
          if( resize ) {
            resize = false;
            if( !tutorial.OnWindowSizeChanged() ) {
              result = false;
              break;
            }
          }
          if( tutorial.ReadyToDraw() ) {
            if( !tutorial.Draw() ) {
              result = false;
              break;
            }
          } else {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
          }
        }
      }

      return result;
    }

#elif defined(VK_USE_PLATFORM_XCB_KHR)

    Window::~Window() {
      xcb_destroy_window( Parameters.Connection, Parameters.Handle );
      xcb_disconnect( Parameters.Connection );
    }

    bool Window::Create( const char *title ) {
      int screen_index;
      Parameters.Connection = xcb_connect( nullptr, &screen_index );

      if( !Parameters.Connection ) {
        return false;
      }

      const xcb_setup_t *setup = xcb_get_setup( Parameters.Connection );
      xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator( setup );

      while( screen_index-- > 0 ) {
        xcb_screen_next( &screen_iterator );
      }

      xcb_screen_t *screen = screen_iterator.data;

      Parameters.Handle = xcb_generate_id( Parameters.Connection );

      uint32_t value_list[] = {
        screen->white_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY
      };

      xcb_create_window(
        Parameters.Connection,
        XCB_COPY_FROM_PARENT,
        Parameters.Handle,
        screen->root,
        20,
        20,
        500,
        500,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        value_list );

      xcb_flush( Parameters.Connection );

      xcb_change_property(
        Parameters.Connection,
        XCB_PROP_MODE_REPLACE,
        Parameters.Handle,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen( title ),
        title );

      xcb_map_window( Parameters.Connection, Parameters.Handle );
      xcb_flush( Parameters.Connection );

      return true;
    }

    bool Window::RenderingLoop( TutorialBase &tutorial ) const {
      // Prepare notification for window destruction
      xcb_intern_atom_cookie_t  protocols_cookie = xcb_intern_atom( Parameters.Connection, 1, 12, "WM_PROTOCOLS" );
      xcb_intern_atom_reply_t  *protocols_reply  = xcb_intern_atom_reply( Parameters.Connection, protocols_cookie, 0 );
      xcb_intern_atom_cookie_t  delete_cookie    = xcb_intern_atom( Parameters.Connection, 0, 16, "WM_DELETE_WINDOW" );
      xcb_intern_atom_reply_t  *delete_reply     = xcb_intern_atom_reply( Parameters.Connection, delete_cookie, 0 );
      xcb_change_property( Parameters.Connection, XCB_PROP_MODE_REPLACE, Parameters.Handle, (*protocols_reply).atom, 4, 32, 1, &(*delete_reply).atom );
      free( protocols_reply );

      // Display window
      xcb_map_window( Parameters.Connection, Parameters.Handle );
      xcb_flush( Parameters.Connection );

      // Main message loop
      xcb_generic_event_t *event;
      bool loop = true;
      bool resize = false;
      bool result = true;

      while( loop ) {
        event = xcb_poll_for_event( Parameters.Connection );

        if( event ) {
          // Process events
          switch (event->response_type & 0x7f) {
            // Resize
          case XCB_CONFIGURE_NOTIFY: {
              xcb_configure_notify_event_t *configure_event = (xcb_configure_notify_event_t*)event;
              static uint16_t width = configure_event->width;
              static uint16_t height = configure_event->height;

              if( ((configure_event->width > 0) && (width != configure_event->width)) ||
                ((configure_event->height > 0) && (height != configure_event->height)) ) {
                resize = true;
                width = configure_event->width;
                height = configure_event->height;
              }
            }
            break;
            // Close
          case XCB_CLIENT_MESSAGE:
            if( (*(xcb_client_message_event_t*)event).data.data32[0] == (*delete_reply).atom ) {
              loop = false;
              free( delete_reply );
            }
            break;
          case XCB_KEY_PRESS:
            loop = false;
            break;
          }
          free( event );
        } else {
          // Draw
          if( resize ) {
            resize = false;
            if( !tutorial.OnWindowSizeChanged() ) {
              result = false;
              break;
            }
          }
          if( tutorial.ReadyToDraw() ) {
            if( !tutorial.Draw() ) {
              result = false;
              break;
            }
          } else {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
          }
        }
      }

      return result;
    }

#elif defined(VK_USE_PLATFORM_XLIB_KHR)

    Window::~Window() {
      XDestroyWindow( Parameters.DisplayPtr, Parameters.Handle );
      XCloseDisplay( Parameters.DisplayPtr );
    }

    bool Window::Create( const char *title ) {
      Parameters.DisplayPtr = XOpenDisplay( nullptr );
      if( !Parameters.DisplayPtr ) {
        return false;
      }

      int default_screen = DefaultScreen( Parameters.DisplayPtr );

      Parameters.Handle = XCreateSimpleWindow(
        Parameters.DisplayPtr,
        DefaultRootWindow( Parameters.DisplayPtr ),
        20,
        20,
        500,
        500,
        1,
        BlackPixel( Parameters.DisplayPtr, default_screen ),
        WhitePixel( Parameters.DisplayPtr, default_screen ) );

      // XSync( Parameters.DisplayPtr, false );
      XSetStandardProperties( Parameters.DisplayPtr, Parameters.Handle, title, title, None, nullptr, 0, nullptr );
      XSelectInput( Parameters.DisplayPtr, Parameters.Handle, ExposureMask | KeyPressMask | StructureNotifyMask );

      return true;
    }

    bool Window::RenderingLoop( TutorialBase &tutorial ) const {
      // Prepare notification for window destruction
      Atom delete_window_atom;
      delete_window_atom = XInternAtom( Parameters.DisplayPtr, "WM_DELETE_WINDOW", false );
      XSetWMProtocols( Parameters.DisplayPtr, Parameters.Handle, &delete_window_atom, 1);

      // Display window
      XClearWindow( Parameters.DisplayPtr, Parameters.Handle );
      XMapWindow( Parameters.DisplayPtr, Parameters.Handle );

      // Main message loop
      XEvent event;
      bool loop = true;
      bool resize = false;
      bool result = true;

      while( loop ) {
        if( XPending( Parameters.DisplayPtr ) ) {
          XNextEvent( Parameters.DisplayPtr, &event );
          switch( event.type ) {
            //Process events
          case ConfigureNotify: {
              static int width = event.xconfigure.width;
              static int height = event.xconfigure.height;

              if( ((event.xconfigure.width > 0) && (event.xconfigure.width != width)) ||
                ((event.xconfigure.height > 0) && (event.xconfigure.width != height)) ) {
                width = event.xconfigure.width;
                height = event.xconfigure.height;
                resize = true;
              }
            }
            break;
          case KeyPress:
            loop = false;
            break;
          case DestroyNotify:
            loop = false;
            break;
          case ClientMessage:
            if( static_cast<unsigned int>(event.xclient.data.l[0]) == delete_window_atom ) {
              loop = false;
            }
            break;
          }
        } else {
          // Draw
          if( resize ) {
            resize = false;
            if( !tutorial.OnWindowSizeChanged() ) {
              result = false;
              break;
            }
          }
          if( tutorial.ReadyToDraw() ) {
            if( !tutorial.Draw() ) {
              result = false;
              break;
            }
          } else {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
          }
        }
      }

      return result;
    }

#endif

  } // namespace OS

} // namespace ApiWithoutSecrets
