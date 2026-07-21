" Smalltalk-76 GuideViewportMorph Class Definition for Vulkan Document Reader "
Object subclass: #GuideViewportMorph
    instanceVariableNames: 'documentIndex selectedDocumentIndex activeSearchQuery expansionState'
    classVariableNames: ''
    poolDictionaries: ''!

!GuideViewportMorph methodsFor: 'initialization'!
initialize
    documentIndex := 642.
    selectedDocumentIndex := 1.
    activeSearchQuery := 'DYSNOMIA'.
    expansionState := 0.!

selectDocument: anIndex
    selectedDocumentIndex := anIndex.!

toggleExpansion: aButtonIndex
    expansionState := expansionState bitXor: (1 bitShift: aButtonIndex).!

evaluateDynamicAddress: aContractAddress
    ^ 'dynamic_' , aContractAddress! !
