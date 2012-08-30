cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Pénurie"
	else
		return "Penury"
	end
end

function getDescription()
	if language == "french" then
		return "Tous les joueurs ont désormais un maximum de 5 sorts en main."
	else
		return "All players have now maximum 5 spells in hand."
	end
end

function onCast()
end

function onResolve()
	attachAsGlobal()
end

function getMod_max_spells()
	return 5
end
