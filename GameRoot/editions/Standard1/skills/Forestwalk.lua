icon = "ane"

function getName()
	if language == "french" then
		return "Forestier"
	else
		return "Forest walk"
	end
end

function getDescription()
	if language == "french" then
		return "L'unité peut se déplacer sur les forêts sans pénalité de mouvement."
	else
		return "The unit can move on forest without move penalty."
	end
end

function getMod_movecost_forest(value)
	return 1
end
