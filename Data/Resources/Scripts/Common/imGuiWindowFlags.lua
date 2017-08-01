local ImGuiWindowFlags = {
    NoTitleBar             = 2 ^ 0,   -- Disable title-bar
    NoResize               = 2 ^ 1,   -- Disable user resizing with the lower-right grip
    NoMove                 = 2 ^ 2,   -- Disable user moving the window
    NoScrollbar            = 2 ^ 3,   -- Disable scrollbars (window can still scroll with mouse or programatically)
    NoScrollWithMouse      = 2 ^ 4,   -- Disable user vertically scrolling with mouse wheel
    NoCollapse             = 2 ^ 5,   -- Disable user collapsing window by double-clicking on it
    AlwaysAutoResize       = 2 ^ 6,   -- Resize every window to its content every frame
    ShowBorders            = 2 ^ 7,   -- Show borders around windows and items
    NoSavedSettings        = 2 ^ 8,   -- Never load/save settings in .ini file
    NoInputs               = 2 ^ 9,   -- Disable catching mouse or keyboard inputs
    MenuBar                = 2 ^ 10,  -- Has a menu-bar
    HorizontalScrollbar    = 2 ^ 11,  -- Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    NoFocusOnAppearing     = 2 ^ 12,  -- Disable taking focus when transitioning from hidden to visible state
    NoBringToFrontOnFocus  = 2 ^ 13,  -- Disable bringing window to front when taking focus (e.g. clicking on it or programatically giving it focus)
    AlwaysVerticalScrollbar= 2 ^ 14,  -- Always show vertical scrollbar (even if ContentSize.y < Size.y)
    AlwaysHorizontalScrollbar= 2 ^ 15,  -- Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    AlwaysUseWindowPadding = 2 ^ 16,  -- Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
    -- [Internal]
    ChildWindow            = 2 ^ 20,  -- Don't use! For internal use by BeginChild()
    ChildWindowAutoFitX    = 2 ^ 21,  -- Don't use! For internal use by BeginChild()
    ChildWindowAutoFitY    = 2 ^ 22,  -- Don't use! For internal use by BeginChild()
    ComboBox               = 2 ^ 23,  -- Don't use! For internal use by ComboBox()
    Tooltip                = 2 ^ 24,  -- Don't use! For internal use by BeginTooltip()
    Popup                  = 2 ^ 25,  -- Don't use! For internal use by BeginPopup()
    Modal                  = 2 ^ 26,  -- Don't use! For internal use by BeginPopupModal()
    ChildMenu              = 2 ^ 27   -- Don't use! For internal use by BeginMenu()
}

return ImGuiWindowFlags
