internal void scene_3d_initialize_gui(Scene3d * scene)
{
	scene->gui 						= {};
	scene->gui.textSize 			= 25;
	scene->gui.textColor 			= colour_white;
	scene->gui.selectedTextColor 	= colour_muted_red;
	scene->gui.padding 				= 10;
	scene->gui.font 				= load_font("c:/windows/fonts/arial.ttf");

	u32 guiTexturePixelColor 		= 0xffffffff;
	TextureAsset guiTextureAsset 	= make_texture_asset(&guiTexturePixelColor, 1, 1, 4);
	scene->gui.panelTexture			= platformApi->push_gui_texture(platformGraphics, &guiTextureAsset);
}

internal void gui_set_cursor_visible(bool menuVisible)
{
	bool windowIsFullscreen = platformApi->is_window_fullscreen(platformWindow);
	bool cursorVisible = !windowIsFullscreen || menuVisible;
	platformApi->set_cursor_visible(platformWindow, cursorVisible);

	logDebug(0) << "Set cursor visible: " << bool_str(cursorVisible);
}


bool32 do_gui(Scene3d * scene, PlatformInput const & input)
{	
	constexpr v2 cornerPosition = {30, 30};
	constexpr v2 centerPosition = {850, 400};


	if (is_clicked(input.start))
	{
		if (scene->menuView == MENU_OFF)
		{
			scene->menuView = MENU_MAIN;
			gui_reset_selection();
			gui_set_cursor_visible(true);
		}
		else if (scene->menuView == MENU_MAIN)
		{
			scene->menuView = MENU_OFF;
			gui_set_cursor_visible(false);
		}
	}

	bool32 keepScene = true;
	
	v4 menuColor = colour_rgb_alpha(colour_bright_blue.rgb, 0.5);
	switch(scene->menuView)
	{	
		case MENU_OFF:
			// Nothing to do
			break;

		case MENU_CONFIRM_EXIT:
		{
			gui_position(cornerPosition);

			gui_start_panel("Exit to Main Menu?", menuColor);

			if (gui_button("Yes"))
			{
				keepScene = false;
			}

			if (gui_button("No"))
			{
				scene->menuView = MENU_MAIN;
				gui_reset_selection();
			}

			gui_end_panel();
		} break;

		case MENU_MAIN:
		{
			gui_position(cornerPosition);	

			gui_start_panel("Menu", menuColor);

			v2 mousePosition = input.mousePosition;

			gui_float_slider_2("X", &mousePosition.x, -20000, 20000);
			gui_float_slider_2("Y", &mousePosition.y, -20000, 20000);

			if (gui_button("Continue"))
			{
				scene->menuView = MENU_OFF;
				gui_set_cursor_visible(false);
			}

			char const * const cameraModeLabels [] =
			{
				"Camera Mode: Player", 
				"Camera Mode: Free",
				"Camera Mode: Mouse",
			};
			
			if (gui_button(cameraModeLabels[scene->cameraMode]))
			{
				scene->cameraMode = (CameraMode)((scene->cameraMode + 1) % CAMERA_MODE_COUNT);
			}

			char const * const debugLevelButtonLabels [] =
			{
				"Debug Level: Off",
				"Debug Level: Player",
				"Debug Level: Player, NPC",
				"Debug Level: Player, NPC, Background"
			};

			if(gui_button(debugLevelButtonLabels[global_DebugDrawLevel]))
			{
				global_DebugDrawLevel += 1;
				global_DebugDrawLevel %= DEBUG_LEVEL_COUNT;
			}

			char const * const drawDebugShadowLabels [] =
			{
				"Debug Shadow: Off",
				"Debug Shadow: On"
			};

			if (gui_button(drawDebugShadowLabels[scene->drawDebugShadowTexture]))
			{
				scene->drawDebugShadowTexture = !scene->drawDebugShadowTexture;
			}

			if (gui_button("Reload Shaders"))
			{
				platformApi->reload_shaders(platformGraphics);
			}

			char const * const timeScaleLabels [scene->timeScaleCount] =
			{
				"Time Scale: 1.0",
				"Time Scale: 0.1",
				"Time Scale: 0.5",
			};

			if (gui_button(timeScaleLabels[scene->timeScaleIndex]))
			{
				scene->timeScaleIndex += 1;
				scene->timeScaleIndex %= scene->timeScaleCount;
			}

			if (gui_button("Save Game"))
			{
				// Todo(Leo): Show some response
				write_save_file(scene);

				scene->menuView = MENU_SAVE_COMPLETE;
				gui_reset_selection();
			}

			if (gui_button("Edit Sky"))
			{
				scene->menuView = MENU_EDIT_SKY;
				gui_reset_selection();
			}

			if (gui_button("Edit Mesh Generation"))
			{
				scene->menuView = MENU_EDIT_MESH_GENERATION;
				gui_reset_selection();
			}

			if (gui_button("Read Settings"))
			{
				read_settings_file(scene->settings);
			}

			if (gui_button("Write Settings"))
			{
				write_settings_file(scene->settings);
			}

			if (gui_button("Exit Scene"))
			{
				scene->menuView = MENU_CONFIRM_EXIT;
				gui_reset_selection();
			}

			// {
			// 	char const * mouseOnStrings[] = {"Mouse: Off", "Mouse: On"};
			// 	local_persist bool32 mouseOn = true;

			// 	if (gui_button(mouseOnStrings[mouseOn]))
			// 	{
			// 		mouseOn = !mouseOn;
			// 		platformApi->set_cursor_visible(platformWindow, mouseOn);
			// 	}
			// }

			gui_end_panel();
		} break;

		case MENU_EDIT_MESH_GENERATION:
		{
			gui_position(cornerPosition);	

			gui_start_panel("Edit Mesh Generation", menuColor);

			gui_float_slider("Grid Scale", &scene->metaballGridScale, 0.2, 1);

			char const * const drawMCStuffTexts [] = 
			{
				"Draw: Off",
				"Draw: On"
			};

			if (gui_button(drawMCStuffTexts[scene->drawMCStuff]))
			{
				scene->drawMCStuff = !scene->drawMCStuff;
			}

			if (gui_button("Back") || is_clicked(input.start))
			{
				scene->menuView = MENU_MAIN;
				gui_reset_selection();
			}

			gui_end_panel();
		} break;
	
		case MENU_SAVE_COMPLETE:
		{
			gui_position(centerPosition);
			gui_start_panel("Game Saved!", menuColor);

			if (gui_button("Ok"))
			{
				scene->menuView = MENU_MAIN;
				gui_reset_selection();
			}

			gui_end_panel();

		} break;

		case MENU_CONFIRM_TELEPORT:
		{
			gui_position(cornerPosition);

			gui_start_panel("Teleport Player Here?", menuColor);

			if (gui_button("Yes"))
			{
				scene->playerCharacterTransform.position = scene->freeCamera.position;
				scene->menuView = MENU_OFF;
				scene->cameraMode = CAMERA_MODE_PLAYER;
			}

			if (gui_button("No"))
			{
				scene->menuView = MENU_OFF;
			}

			gui_end_panel();
		} break;

		case MENU_EDIT_SKY:
		{
			gui_position(cornerPosition);

			gui_start_panel("Edit Sky", menuColor);

			auto mark_dirty = [scene](bool dirty)
			{
				scene->settings.dirty = scene->settings.dirty || dirty;
			};

			mark_dirty(gui_float_slider("Sky Color", &scene->settings.skyColourSelection.value_f32, 0,1));

			char const * const colorFromTreeDistanceTexts [] = 
			{
				"Color From Tree Distance: Off",
				"Color From Tree Distance: On"
			};

			if (gui_button(colorFromTreeDistanceTexts[scene->getSkyColorFromTreeDistance]))
			{
				scene->getSkyColorFromTreeDistance = !scene->getSkyColorFromTreeDistance;
			}

			mark_dirty(gui_float_slider("Sky Height Angle", &scene->settings.sunHeightAngle.value_f32, -1, 1));
			mark_dirty(gui_float_slider("Sky Orbit Angle", &scene->settings.sunOrbitAngle.value_f32, 0, 1));

			gui_color("Sky Gradient Bottom", &scene->skyBottomColor, GUI_COLOR_FLAGS_HDR);
			gui_color("Sky Gradient Top", &scene->skyTopColor, GUI_COLOR_FLAGS_HDR);

			gui_color("Horizon Halo Color", &scene->horizonHaloColor, GUI_COLOR_FLAGS_HDR);
			gui_float_slider("Horizon Halo Falloff", &scene->horizonHaloFalloff, 0.0001, 1);

			gui_color("Sun Halo Color", &scene->sunHaloColor, GUI_COLOR_FLAGS_HDR);
			gui_float_slider("Sun Halo Falloff", &scene->sunHaloFalloff, 0.0001, 1);

			// {
			// 	local_persist v4 color = {2,2,2,1};

			// 	bool modified = gui_color("Test Color", &color, GUI_COLOR_FLAGS_HDR);

			// 	if (modified)
			// 	{
			// 		scene->niceSkyGradient.colours[3] = color;

			// 		TextureAsset niceSkyGradientAsset 	= generate_gradient(*global_transientMemory, 128, scene->niceSkyGradient);
			// 		niceSkyGradientAsset.addressMode 	= TEXTURE_ADDRESS_MODE_CLAMP;

			// 		platformApi->update_texture(platformGraphics, scene->niceSkyGradientTexture, &niceSkyGradientAsset);

			// 		logDebug(0) << "Color = " << color;
			// 	}
			// }

			mark_dirty(gui_float_slider("HDR Exposure", &scene->settings.hdrExposure.value_f32, 0.1, 10));
			mark_dirty(gui_float_slider("HDR Contrast", &scene->settings.hdrContrast.value_f32, -1, 1));

			if (gui_button("Back") || is_clicked(input.start))
			{
				scene->menuView = MENU_MAIN;
			}

			gui_end_panel();

		} break;
	}

	// gui_position({100, 100});
	// gui_text("Sphinx of black quartz, judge my vow!");
	// gui_text("Sphinx of black quartz, judge my vow!");

	// gui_pivot(GUI_PIVOT_TOP_RIGHT);
	// gui_position({100, 100});
	// gui_image(shadowTexture, {300, 300});

	gui_position({1550, 30});
	if (scene->drawDebugShadowTexture)
	{
		gui_image(GRAPHICS_RESOURCE_SHADOWMAP_GUI_TEXTURE, {300, 300});
	}

	gui_end_frame();

	return keepScene;
}