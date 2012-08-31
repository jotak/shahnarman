icon = "mountainwalk"

function getName()
	if language == "french" then
		return "Montagnard"
	else
		return "Mountain walk"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité peut se déplacer sur les montagnes sans pénalité de mouvement."
	else
		return "The unit can move on mountains without move penalty."
	end
end

function getMod_movecost_mountain(value)
	return 1
end
