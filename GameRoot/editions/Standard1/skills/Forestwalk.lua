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
		return "L'unit� peut se d�placer sur les for�ts sans p�nalit� de mouvement."
	else
		return "The unit can move on forest without move penalty."
	end
end

function getMod_movecost_forest(value)
	return 1
end
