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
		return "L'unit� peut se d�placer sur les montagnes sans p�nalit� de mouvement."
	else
		return "The unit can move on mountains without move penalty."
	end
end

function getMod_movecost_mountain(value)
	return 1
end
