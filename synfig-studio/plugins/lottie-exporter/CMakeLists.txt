set(INSTALL_DESTINATION "share/synfig/plugins/lottie-exporter")
set(PLUGIN_LOTTIE_EXPORTER_FILES
    bodymovin.js
    canvas.py
    lottie-exporter.py
    settings.py
    test_bodymovin.js
    export_without_variable_width.py
    
    # common
    common/Bline.py
    common/Canvas.py
    common/Color.py
    common/Count.py
    common/Gradient.py
    common/Hermite.py
    common/Layer.py
    common/Matrix2.py
    common/misc.py
    common/Param.py
    common/Vector.py

    # effects
    effects/allmask.py
    effects/color.py
    effects/controller.py
    effects/feather.py
    effects/fill.py
    effects/fillmask.py
    effects/invert.py
    effects/opacity.py
    effects/point.py
    effects/slider.py

    # helpers
    helpers/bezier.py
    helpers/blendMode.py
    helpers/mask.py
    helpers/transform.py

    # layers
    layers/blur.py
    layers/driver.py
    layers/group.py
    layers/image.py
    layers/preComp.py
    layers/rotate_layer.py
    layers/scale_layer.py
    layers/shape.py
    layers/shape_solid.py
    layers/solid.py
    layers/text.py
    layers/translate_layer.py

    # properties
    properties/multiDimensionalKeyframed.py
    properties/offsetKeyframe.py
    properties/shapeKeyframed.py
    properties/timeAdjust.py
    properties/value.py
    properties/valueKeyframe.py
    properties/valueKeyframed.py

    # properties/shapePropKeyframe
    properties/shapePropKeyframe/circle.py
    properties/shapePropKeyframe/constant_width_outline.py
    properties/shapePropKeyframe/helper.py
    properties/shapePropKeyframe/outline.py
    properties/shapePropKeyframe/polygon.py
    properties/shapePropKeyframe/rectangle.py
    properties/shapePropKeyframe/region.py
    properties/shapePropKeyframe/star.py

    # shapes
    shapes/circle.py
    shapes/fill.py
    shapes/gFill.py
    shapes/rectangle.py
    shapes/shape.py
    shapes/star.py

    # sources
    sources/image.py
    sources/precomp.py

    # synfig
    synfig/animation.py
    synfig/group.py
    synfig/rectangle.py
)

foreach(CURRENT_FILE ${PLUGIN_LOTTIE_EXPORTER_FILES})
    get_filename_component(CURRENT_DIRECTORY ${CURRENT_FILE} DIRECTORY)
    file(COPY ${CURRENT_FILE}
        DESTINATION ${SYNFIG_BUILD_ROOT}/${INSTALL_DESTINATION}/${CURRENT_DIRECTORY}
    )

    install(FILES ${CURRENT_FILE}
        DESTINATION ${INSTALL_DESTINATION}/${CURRENT_DIRECTORY}
    )
endforeach()

STUDIO_INTLTOOL_MERGE(
    TARGET_NAME plugin_lottie_exporter_xml
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/plugin.xml.in
    INSTALL_DESTINATION ${INSTALL_DESTINATION}
)
